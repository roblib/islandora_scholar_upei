<?php

/*
 * @file
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
    $title = "";
    $headers = variable_get('scholar_solr_node_for_header', 'Institutional Repository Collection ~ 8');
    $arr = islandora_build_substitution_list($headers);
    //load a node as a header for the search results in case we have canned searches we would like to explain
    $node_id = $arr[$solrQueryProcessor->solrQuery];
    if (!empty($node_id)) {
      $output .= '<div id="islandora_search_header">';
      $node = node_load($node_id);
      $output .= $node->body;
      $output .= '</div>';
    }
    $limit_block = module_invoke('islandora_solr_search', 'block', 'view', 'limit_results');

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

    //$output .= '<h2>' . $title . '</h2>';
    $output .= '<div class="islandora-solr-search-controls">';
    $output .= '<div class="islandora-solr-search-count-limit">' . $limit_block['content'] . '</div>';
    $output .= '<div class="islandora-solr-result-count"><div id="islandora_solr_result_count">' . ($solrQueryProcessor->solrStart + 1) . ' - ' . $end . ' of ' . $total . '</div></div>';
    $output .= '<div class="islandora-solr-search-pager-top">' . theme('pager', NULL, $solrQueryProcessor->solrLimit, 0, NULL, 5) . '</div>';
    $output .= '</div>';
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

    $output .= $this->printResultsScholar($solrQueryProcessor->solrResult);

    // Render Drupal pager (bottom of search results)
    $output .= theme('pager', NULL, $solrQueryProcessor->solrLimit, 0, NULL, 5);


    $output .= "</div>"; // class = "content"
    return $output;
  }

  function printResultsScholar($results) {
    // drupal_add_js(drupal_get_path('module', 'scholar') . '/js/searchresults.js');
    module_load_include('inc', 'fedora_repository', 'CollectionClass');
    module_load_include('inc', 'citeproc', 'CiteProcJSBibliography');
    module_load_include('inc', 'scholar', '/citation/bibliography/BibliographyForm');
    module_load_include('inc', 'scholar', '/citation/bibliography/CitationForm');
    module_load_include('inc', 'fedora_repository', '/api/fedora_item');
    module_load_include('inc', 'scholar', 'Coins');
    module_load_include('inc', 'islandora_bibliography', 'Bibliography');
    drupal_add_css(drupal_get_path('module', 'islandora_bibliography') . '/css/export.css');
    drupal_add_css(drupal_get_path('module', 'scholar') . '/css/scholar.css');

    static $path;

    $path = drupal_get_path('module', 'scholar');
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
          
        }
      }
    }

    global $base_url;
    global $base_path;
    if (empty($results)) {
      return "no results";
    }

    $row = 1;

    foreach ($results->response->docs as $doc) {
      $rows = array();
      $csl = variable_get('scholar_default_csl', 'default');
      $department = $person = FALSE;
      foreach ($doc as $field => $value) {
        if ($field == 'rels.isMemberOfCollection' && $value == 'ir:people') {
          $person = TRUE;
        }
        elseif ($field == 'rels.isMemberOfCollection' && $value == 'ir:departments') {
          $department = TRUE;
        }
      }

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
      $dc_title = str_replace(array('/', ':', '-', ' and ', ' or ', ' of ', ' in ', ' on ', ' to ', ' for ', ' with ', ' the ', 'The ', ' but ', ' not ', ' by '), ' ', $dc_title);
      if (is_array($dc_title)) {
        $dc_title = $dc_title[0];
      }
      if (strlen($dc_title) > 60) {
        $array = explode(' ', $dc_title);
        $dc_title = implode(' ', array_slice($array, 0, 9));
      }
      if ($mods) {
        $object = new Fedora_Item($doc->PID);
        $ds_array = $object->get_datastreams_list_as_array();
        $pdf_check = array_key_exists('PDF', $ds_array) || array_key_exists('OBJ', $ds_array);
        $check = '';
        $data = new Coins($doc->PID);
        if (array_key_exists('OBJ', $ds_array) && $data->status == 'FULL_TEXT_AVAILABLE') {//check status in case ds exists but is embargoed
          $check = 'OBJ';
        }
        if (array_key_exists('PDF', $ds_array) && $data->status == 'FULL_TEXT_AVAILABLE') {
          $check = 'PDF';
        }
        $url_frag = 'fedora/repository/';
        if (module_exists('scholar_tracking')) {
          $url_frag = 'download_ds/';
        }

        switch ($check) {
          case 'OBJ':
            $download_img = $base_url . '/' . $path . '/images/download.png';
            $pdf_result = '<div class="scholar_pdf button">' . l('Full text', $url_frag . $doc->PID . '/OBJ/OBJ', array('html' => TRUE)) . '</div>';
            break;
          case 'PDF':
            $download_img = $base_url . '/' . $path . '/images/download.png';
            $pdf_result = '<div class="scholar_pdf button">' . l('Full text', $url_frag . $doc->PID . '/PDF/PDF', array('html' => TRUE)) . '</div>';
            break;
          default:
            $coins_url = '&#10;&#9;&#9;&#9;&#9;http://articles.library.upei.ca:7888/godot/hold_tab.cgi?hold_tab_branch=PCU&amp;issn=' . $data->issn . '&date=' . $data->date . '&volume=' . $data->volume . '&issue=' . $data->issue . '&spage=' . $data->start_page . '&atitle=' . $data->title . '&stitle=' . $data->journal_title;
            $pdf_result = '<div class="scholar_pdf"><A HREF="' . $coins_url . '"><img src="http://library.upei.ca/sites/all/files/sharedfolder/files/getit_upei.gif" /> </A></div>';
            break;
        }

        $supp_result = NULL;

        if (array_key_exists('ZIP', $ds_array)) {
          $supp_result = '<div class="scholar_pdf">' . l('<img src="' . $download_img . '"/> Supplemental information', 'fedora/repository/' . $doc->PID . '/ZIP/ZIP', array('html' => TRUE)) . '</div>';
        }

        $abstract_snippet = 600;
        if (strlen($data->abstract) > $abstract_snippet) {
          $abstract = substr($data->abstract, 0, $abstract_snippet);
          $abstract .= '...';
        }
        else {
          $abstract = $data->abstract;
        }

        $scholar_class = Bibliography::Exists($doc->PID) ? 'my-list-remove' : 'my-list-add';
        $link_text = Bibliography::Exists($doc->PID) ? t('Remove from My List') : t('Add to My List');
        //$bookmark = '<div class="scholar_search_bookmark "><a  NAME = "'.$doc->PID.'" id="my-list-link-' .$doc->PID . '" class="'. $scholar_class .' button">'.$link_text.'</a></div>';
        $bibliography_text = ""; //Bibliography::Exists($doc->PID) ? 'Remove from My List' : 'Add to My List';
        $object_title = '<div class="scholar_search_title">' . l($data->title, 'fedora/repository/' . $doc->PID, array('html' => TRUE)) . '</div>';
        $bookmark = '<div class="scholar_search_bookmark ' . $scholar_class . ' ">' . drupal_get_form('islandora_bibliography_citation_search_form', $doc->PID) . '</div><div class="scholar_search_bookmark_text">' . $bibliography_text . '</div>';
        $related = '<div class="scholar_search_related">' . l('<img src="' . $base_url . '/' . $path . '/images/reload.png"/> Related content', 'islandora/solr/search/' . str_replace(' ', ' OR ', $dc_title), array('html' => TRUE)) . '</div>';
        $rows[][] = $object_title . '<div class="scholar_search_citation">' . $bibliography->render() . '</div>' . '<div class="scholar_search_citation">' . $abstract . '</div>' . $bookmark . $pdf_result . $supp_result;

        $row++;
      }
      elseif ($person) {
        $image_path = $base_path . drupal_get_path('module', 'scholar') . '/images/gnome_stock_person.png';
        $rows[][] = '<img src="' . $image_path . '"/>';
        foreach ($doc as $field => $value) {
          $person_result_fields = array('mads.fullname', 'mads.topic');
          if (in_array($field, $person_result_fields)) {
            if ($limitResults && empty($this->resultFieldArray[$field])) {
              continue;
            }
            $translated_field_name = isset($this->allSubsArray[$field]) ? $this->allSubsArray[$field] : $field;
            $rows[$row][] = array(
              'data' => $translated_field_name,
              'header' => TRUE,
            );

            if ($field == 'PID') {
              $l = l($value, 'fedora/repository/' . htmlspecialchars($value, ENT_QUOTES, 'utf-8')) . '</BR>';
              $rows[$row][] = $l;
            }
            elseif ($field == 'Title' && is_array($value)) {
              $rows[$row][] = ucfirst(ltrim($value[0]));
            }
            elseif ($field == 'mads.fullname' && $value != NULL) {
              $l = l($value, 'fedora/repository/' . $doc->PID) . '</BR>';
              $rows[$row][] = $l;
            }
            elseif ($field == 'mads.topic' && is_array($value)) {
              $rows[$row][] = implode(', ', array_unique($value));
            }
            else {
              $rows[$row][] = ucfirst(ltrim($value));
            }
            $row++;
          }
        }
      }
      elseif ($department) {
        $image_path = $base_path . drupal_get_path('module', 'scholar') . '/images/gnome_stock_dept.png';
        $rows[][] = '<img src="' . $image_path . '"/>';
        foreach ($doc as $field => $value) {
          $person_result_fields = array('mads.identifier');
          if (in_array($field, $person_result_fields)) {
            if ($limitResults && empty($this->resultFieldArray[$field])) {
              continue;
            }
            $translated_field_name = isset($this->allSubsArray[$field]) ? $this->allSubsArray[$field] : $field;
            $rows[$row][] = array(
              'data' => $translated_field_name,
              'header' => TRUE,
            );

            if ($field == 'PID') {
              $l = l($value, 'fedora/repository/' . htmlspecialchars($value, ENT_QUOTES, 'utf-8')) . '</BR>';
              $rows[$row][] = $l;
            }
            elseif ($field == 'Title' && is_array($value)) {
              $rows[$row][] = ucfirst(ltrim($value[0]));
            }
            elseif ($field == 'mads.identifier' && $value != NULL) {
              $l = l($value, 'fedora/repository/' . $doc->PID) . '</BR>';
              $rows[$row][] = $l;
            }
            else {
              $rows[$row][] = ucfirst(ltrim($value));
            }
            $row++;
          }
        }
      }
      else {
        foreach ($doc as $field => $value) {
          if ($limitResults && empty($this->resultFieldArray[$field])) {
            continue;
          }

          $translated_field_name = isset($this->allSubsArray[$field]) ? $this->allSubsArray[$field] : $field;
          $rows[$row][] = array(
            'data' => $translated_field_name,
            'header' => TRUE,
          );

          if ($field == 'PID') {
            $l = l($value, 'fedora/repository/' . htmlspecialchars($value, ENT_QUOTES, 'utf-8')) . '</BR>';
            $rows[$row][] = $l;
          }
          elseif ($field == 'Title' && is_array($value)) {
            $rows[$row][] = ucfirst(ltrim($value[0]));
          }
          else {
            $rows[$row][] = ucfirst(ltrim($value));
          }
          $row++;
        }
      }

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
    $_SESSION['search_url'] = request_uri();
    return implode('', $items);
  }

}