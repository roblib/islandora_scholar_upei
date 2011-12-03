<?php

/*
 * contains methods to search solr and display results.  depends on Apache_Solr_Php client.
 */

module_load_include('inc', 'islandora_solr_search', 'IslandoraSolrResults');

/**
 * Extention of IslandoraSolrResults for templating purposes.
 * This overrides the displayResults function to provide an alternate display type.
 */
class ScholarSolrResults extends IslandoraSolrResults {

  static $facetSeparator = '~'; //used to separate facets in url
  static $slashReplacement = '~slsh~'; // a pattern used to replace / in url's the slash breaks drupal clean url's
  public static $SEARCH_CLASS_ADVANCED_SEARCH_NUMBER_FIELDS = 5;

  function IslandoraSolrResultsTable() {
    parent::__construct();
  }

  /**
   * Outputs results in unified tabular format.
   *
   * This function collects all field names from all results in the 
   * resultset and constructs a single table with said field names in 
   * a unified header row, rather than outputting each result as a 
   * miniature horizontal table.
   *
   * @param $results
   *   A solr resultset object.
   *
   * @return
   *   html output for the resultset. Note: we currently create this 
   *   output manually, should refactor to use drupal forms api.
   */
  function displayResultsScholar($solrQueryProcessor, $title = "Search Results", $output = '') {

    $apacheSolrResult = $solrQueryProcessor->solrResult;
    $total = (int) $apacheSolrResult->response->numFound;
    $end = min(($solrQueryProcessor->solrLimit + $solrQueryProcessor->solrStart), $total);

    // Check for empty resultset
    if ($total === 0) {
      $output = "<h2>Search Results</h2>";
      $output .= "<div>Sorry, but your search returned no results.</div>";
      return $output;
    }

    // Initialize drupal-style pager
    islandora_solr_search_pager_init($total, $solrQueryProcessor->solrLimit);

    // Get secondary display profiles
    $secondary_display_profiles = module_invoke_all('islandora_solr_secondary_display');
    // How about this for an expected return?
    // return array(
    // 'machine-name' = array(
    //   'name' => 'Human Readable Name',
    //   'module' => 'module_name',
    //   'file' => 'FileName.inc',
    //   'class' => 'ClassName',
    //   'function' => 'function_name',
    //   'description' => 'A description of the display profile',
    //   'logo' => 'Some text, some <html>, an <img>, etc used to link to this output,
    // );
    // check if
    // if the islandora_solr_search admin form has never been saved, then
    // the variable 'islandora_solr_secondary_display' won't be set yet.
    $secondary_array = variable_get('islandora_solr_secondary_display', array());

    // count number of elements in array
    $array_count = count($secondary_array);
    // count number of disabled displays
    $i = 0;
    foreach ($secondary_array as $key => $value) {
      if ($value === 0) {
        $i++;
      }
    }

    // if number of elements is the same as the number of disabled displays,
    // this means that there are no displays enabled. 
    if ($array_count != $i) {

      $output .= "<div id='islandora_solr_secondary_display_profiles'>";

      foreach ($secondary_array as $name => $status) {
        if ($status === $name) {

          $description = $secondary_display_profiles[$name]['description'];
          $logo = $secondary_display_profiles[$name]['logo'];

          $attributes = array('title' => $description);
          $query = array();
          $query[] = drupal_query_string_encode(array('solr_profile' => $name));
          $output .= l($logo, $_GET['q'], array('attributes' => $attributes, 'html' => TRUE, 'query' => count($query) ? implode('&', $query) : NULL));
        }
      }

      $output .= "</div>";
    }

    $output .= '<h2>' . $title . '</h2>';
    $output .= '<div id="islandora_solr_result_count">(' . ($solrQueryProcessor->solrStart + 1) . ' - ' . $end . ' of ' . $total . ')</div>';
    $output .= '<div class="content">';

//    drupal_set_message(print_r($secondary_display_profiles,1));

    /*
     * Then, here, we need to build links to the current query under the secondary query display
     * profile for each such profile which uses the hook.
     *
     * The link should basically be $theCurrentURL with &proile=<profile_name> tacked on.
     *
     * Also, perhaps there's no difference between primary and secondary profiles?
     */

    $output .= $this->printResultsChem($solrQueryProcessor->solrResult);

    // Render Drupal pager (bottom of search results)
    $output .= theme('pager', NULL, $solrQueryProcessor->solrLimit, 0, NULL, 5);


    $output .= "</div>"; // class = "content"
    return $output;
  }

  function printResultsChem($results) {

    module_load_include('inc', 'fedora_repository', 'CollectionClass');
    module_load_include('inc', 'citeproc', 'CiteProcJSBibliography');
    module_load_include('inc', 'scholar', '/citation/bibliography/BibliographyForm');
    module_load_include('inc', 'scholar', '/citation/bibliography/CitationForm');
    drupal_add_css(drupal_get_path('module', 'islandora_bibliography') . '/css/export.css');
    drupal_add_css(drupal_get_path('module', 'scholar') . '/scholar.css');

    $items = array();
    $type = "ol";
    $title = NULL;
    $recordStart = $results->response->start;
    $limitResults = variable_get('islandora_solr_search_limit_result_fields', 0);
    $highlights = $results->highlighting;
    foreach ($highlights as $highlight) {
      $fields = get_object_vars($highlight);
      $keys = array_keys($fields);
      if (count($keys) > 0) {
        foreach ($highlight->$keys[0] as $body) {
          //drupal_set_message("$keys[0]  $body");
        }
      }
      //drupal_set_message($highlight->$keys[0]);
    }

    global $base_url;
    if (empty($results)) {
      return "no results";
    }

    $row = 1;

    foreach ($results->response->docs as $doc) {
      $rows = array();

      $csl = 'APA';

      $style = citeproc_style($csl);
      $bibliography = new CiteProcJSBibliography($style);
      $mods = islandora_bibilgraphy_form_get_mods($doc->PID);
      if ($mods) {
        $ref = citeproc_citation_from_mods($mods);
        $bibliography->addCitation($ref);
      }
      $dc_attribute = 'dc.title';
      $dc_title = $doc->$dc_attribute;
      $dc_title = preg_replace('/\s+/', ' ', $dc_title);
      $dc_title = str_replace(array('/', ':'), array(' ', ' '), $dc_title);
      if (is_array($dc_title)) {
        $dc_title = $dc_title[0];
      }      
      if (strlen($dc_title) > 60) {
//        $dc_title = substr($dc_title, 0, 60);
        $array = explode(' ', $dc_title);
        $dc_title = implode(' ', array_slice($array, 0, 9));
      }
      $number = '<div class="scholar_search_number">' . l($row, 'fedora/repository/' . $doc->PID) . '</div>';
      $bookmark = '<div class="scholar_search_bookmark">' . drupal_get_form('islandora_bibliography_citation_search_form', $doc->PID) . '</div>';
      $related = '<div class="scholar_search_related">' . l('Related articles', 'islandora/solr/search/' . $dc_title) . '</div>';
      $rows[][] = $number . '<div class="scholar_search_citation">' . $bibliography->render() . '</div>' . $bookmark . $related;
      $row++;
      $items[] = theme_table(NULL, $rows, array('class' => 'islandora_solr_search_results_object'));
    }
    $output = theme_item_list($items, $title, $type, array('class' => 'stuff', 'start' => ($recordStart + 1)));

    if (variable_get('islandora_solr_search_debug_mode', 0)) { // debug dump
      $results_r .= "<pre>Results: " . print_r($results, TRUE) . "</pre>";
      $fieldset_r = array(
        '#title' => t("Raw Results"),
        '#collapsible' => TRUE,
        '#collapsed' => TRUE,
        '#value' => $results_r,
      );

      $output .= theme('fieldset', $fieldset_r);
    }
    $_SESSION['search_url'] = $_SERVER['REQUEST_URI'];
    return implode('', $items);
  }

}