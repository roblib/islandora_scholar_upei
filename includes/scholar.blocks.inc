<?php

/*
 * @file
 */

class ScholarSearchResults {

  /**
   * Function: currentQuery
   *
   * Description: Displays elements of the current solr query (with enabled filters).
   *   
   *
   * @param IslandoraSolrQueryProcessor $solrQueryProcessor
   */
  function currentQuery($solrQueryProcessor) {
    drupal_add_css(drupal_get_path('module', 'scholar') . '/css/searchresults.css');
    // set path
    $path = SOLR_SEARCH_PATH . '/' . $solrQueryProcessor->solrQuery; // $_GET['q'] didn't seem to work here
    // get date format
    $format = variable_get('islandora_solr_facet_date_format', 'Y');

    // set output variable
    $output = '';

    // get user provided filter parameters
    $fq = $solrQueryProcessor->internalSolrParams['f'];

    // parameters set in url
    $params = $solrQueryProcessor->internalSolrParams;


    // Get Query values
    if (!in_array($solrQueryProcessor->solrQuery, $solrQueryProcessor->different_kinds_of_nothing)) {
      // get query value
      $query_value = $solrQueryProcessor->solrQuery;

      // set list variables
      $query_list = array();

      // remove link keeps all parameters (query gets removed instead)
      $query_minus = array();
      $query_minus = $params;

      // remove query from path
      $path_minus = implode('/', explode('/', $path, -1)) . '/ ';

      // set attributes variable for remove link
      $attr_minus = array();
      // set title
      $attr_minus['title'] = t('Remove') . ' ' . $query_value;
      // set rel
      $attr_minus['rel'] = 'nofollow';
      // set url
      $attr_minus['href'] = url($path_minus, array('query' => $query_minus));

      // create link
      // we're not using l() because of active classes: http://drupal.org/node/41595
      $query_list[] = $query_value . ' <i class="islandora-solr-search-query-x-icon"><a' . drupal_attributes($attr_minus) . '>[x]</a></i>';

      // add wrap and list
      $output .= '<div class="islandora-solr-search-query-inline clear-block">';
      //TODO make this a template
      $output .= '<dl><dt>Your Search:</dt><dd>';

      $output .= theme_item_list($query_list, NULL, "ul", array('class' => 'islandora_solr_search_query_list query_list'));
      $output .= '</dd>';
      // 'islandora_solr_search_query_wrap'
    }
    else {
      $output .= '<div class="islandora-solr-search-query-inline clear-block">';
      //TODO make this a template
      $output .= '<dl><dt>Your Search:</dt><dd>';

      $output .= 'no term entered';
      $output .= '</dd>';
    }


    // Get Filter values
    if (!empty($fq)) {

      // set list variables
      $filter_list = array();

      // loop over filters
      foreach ($fq as $key => $filter) {

        // check for exclude filter
        if ($filter[0] == '-') {
//          $symbol = '&#2260;';
          $symbol = '&ne;';
        }
        else {
          $symbol = '=';
        }

        // split the filter into field and value
        $filter_split = explode(':', $filter, 2);
        // trim brackets
        $filter_split[1] = trim($filter_split[1], "\"");

        // if value is date
        if (isset($solrQueryProcessor->solrParams['facet.date']) AND in_array(ltrim($filter_split[0], '-'), $solrQueryProcessor->solrParams['facet.date'])) {
          // split range filter string to return formatted date values
          $filter_str = $filter_split[1];
          $filter_str = trim($filter_str, '[');
          $filter_str = trim($filter_str, ']');
          $filter_array = explode(' TO ', $filter_str);
          $filter_split[1] = format_date(strtotime(trim($filter_array[0])) + (60 * 60 * 24), 'custom', $format) . ' - ' . format_date(strtotime(trim($filter_array[1])) + (60 * 60 * 24), 'custom', $format);
        }

        // pull out filter (for exclude link)
        $query_minus = array();
        $f_x['f'] = array_diff($params['f'], array($filter));
        $query_minus = array_merge($params, $f_x);
        // resetting the filter key's order // @TODO find a cleaner way to do this.
        if ($query_minus['f']) {
          $query_minus['f'] = array_merge(array(), $query_minus['f']);
        }
        // remove 'f' if empty
        if (empty($query_minus['f'])) {
          unset($query_minus['f']);
        }

        // set attributes variable for remove link
        $attr_minus = array();
        // set title
        $attr_minus['title'] = t('Remove') . ' ' . $filter;
        // set rel
        $attr_minus['rel'] = 'nofollow';
        // set url
        $attr_minus['href'] = url($path, array('query' => $query_minus));

        // create link
        // we're not using l() because of active classes: http://drupal.org/node/41595
        $filter_list[] = $filter_split[1] . ' <i class="islandora-solr-search-query-x-icon"><a' . drupal_attributes($attr_minus) . '>[x]</a></i> ';
      }

      // return filter list
      $output .= '<dt>Limited to:</dt>';
      $output .= '<dd>';
      // $output .= '<div class="islandora_solr_search_filter_wrap">';
      $output .= theme_item_list($filter_list, NULL, "ul", array('class' => 'islandora_solr_search_filter_list filter_list'));
      $output .= '</dd>'; // 'islandora_solr_search_filter_wrap'
    }
    $output .= '</dl></div>';
    return $output;
  }

}

/**
 *
 * @param array $form_state 
 */
function scholar_extended_search_form_build_query(array &$form_state) {
  $statements = array();
  foreach ($form_state['values']['terms'] as $term) {
    $field = $term['field'];
    $search = trim($term['search']);
    if (!empty($search)) {
      $search = scholar_escape_phrase($search);
      $statements[] = "$field:($search)";
    }
  }
  $query = !empty($statements) ? implode(' AND ', $statements) : '*:*'; // Empty return all results. 
  return $query;
}

function scholar_escape($value)
	{
		//list taken from http://lucene.apache.org/java/docs/queryparsersyntax.html#Escaping%20Special%20Characters
		$pattern = '/(\+|-|&&|\|\||!|\(|\)|\{|}|\[|]|\^|"|~|\*|\?|:|\\\)/';
		$replace = '\\\$1';

		return preg_replace($pattern, $replace, $value);
	}

/**
 * Escape a value meant to be contained in a phrase for special query characters
 *
 * @param string $value
 * @return string
 */
function scholar_escape_phrase($value) {
  $pattern = '/("|\\\)/';
  $replace = '\\\$1';

  return preg_replace($pattern, $replace, $value);
}

function scholar_recent_items($form_state) {
  module_load_include('inc', 'fedora_repository', 'api/fedora_utils');
  module_load_include('inc', 'fedora_repository', 'api/fedora_item');
  module_load_include('inc', 'Fedora_Repository', 'CollectionClass');
  $query_string = 'select $object $modified from <#ri> where $object <fedora-model:hasModel> <info:fedora/islandora:citationCModel> and $object <fedora-view:lastModifiedDate> $modified order by $modified desc;';
  $query_string = htmlentities(urlencode($query_string));
  $content = '';
  $url = variable_get('fedora_repository_url', 'http://localhost:8080/fedora/risearch');
  $url .= "?type=tuples&flush=TRUE&format=Sparql&limit=3&offset=0&lang=itql&stream=on&query=" . $query_string;
  $content .= do_curl($url);
  if ($content == '') {
    drupal_set_message(t('The fedora server is not responding!'));
    return FALSE;
  }
  $result = new SimpleXMLElement($content);

  $entries = $result->results->result;

  $list = '<ul>';

  foreach ($entries as $entry) {
    $pid = explode('/', $entry->object['uri']);
    $object = new Fedora_Item($pid[1]);
    $history = $object->get_datastream_history('DC');
    if (is_array($history)) {
      $created = $history[0]->createDate;
    }
    else {
      $created = $history->createDate;
    }
    $xmlstr = $object->get_datastream_dissemination('MODS');
    if ($xmlstr) {
      $xml = new SimpleXMLElement($xmlstr);
      $list .= '<li>' . l($xml->titleInfo->title, 'fedora/repository/' . $pid[1], array('html' => TRUE)) . '</li>';
    }
    else {
      $list .= '<li>' . l(t('Title not found'), 'fedora/repository/' . $pid[1]) . '</li>';
    }
  }

  $list .= '</ul>';
  $form = array();

  $form['title'] = array(
    '#value' => $list,
  );

  return $form;
}

function scholar_total_records($form_state) {

  $solr_location = variable_get('islandora_solr_search_block_url', 'localhost:8080/solr');

  $query = @file_get_contents('http://' . $solr_location . '/select/?q=PID:ir*%20AND%20rels.hasModel:islandora\:citationCModel&start=0&rows=0&indent=on');
  if ($query != NULL) {
    $xml = new SimpleXMLElement($query);
    $number_of_records = $xml->xpath('./result[@name="response"]/@numFound');
    $form['records'] = array(
      '#value' => t('@records records added', array('@records' => number_format((string) $number_of_records[0]))),
    );
  }

  return $form;
}

function scholar_total_text_downloads($form_state) {
  $query = db_query("select count(pid) from {islandora_scholar_ds_tracking} where dsid = 'PDF' OR dsid = 'OBJ';");
  $count = db_result($query);

  $form['full-text_downloads'] = array(
    '#value' => t('@downloads full-text downloads', array('@downloads' => number_format($count))),
  );

  return $form;
}

function scholar_total_citation_views($form_state) {
  $query = db_query("select count(pid) from {islandora_scholar_page_tracking}");
  $count = db_result($query);

  $form['citation_views'] = array(
    '#value' => t('@views citation views', array('@views' => number_format($count))),
  );

  return $form;
}

function scholar_top_downloads($form_state) {
  
}

function scholar_top_viewed($form_state) {
  module_load_include('inc', 'fedora_repository', 'api/fedora_item');
  $query = db_query("select pid, count(*) from {islandora_scholar_page_tracking} group by pid order by count(*) DESC");

  while (db_fetch_array($query)) {
    $results[] = db_fetch_array($query);
  }

  $results = array_slice($results, 0, 3);

  $content = '<ul>';
  foreach ($results as $result) {
    $pid = $result['pid'];
    $count = $result['count(*)'];
    $object = new Fedora_Item($pid);
    $xmlstr = $object->get_datastream_dissemination('MODS');
    if ($xmlstr) {
      $xml = new SimpleXMLElement($xmlstr);
      $content .= '<li>' . l($xml->titleInfo->title, 'fedora/repository/' . $pid, array('html' => TRUE)) . ' <i>(' . number_format($count) . ')</i></li>';
    }
    else {
      $content .= '<li>' . l(t('Title not found'), 'fedora/repository/' . $pid) . '</li>';
    }
  }

  $content .= '</ul>';

  $form['citation_views'] = array(
    '#value' => $content,
  );

  return $form;
}

function scholar_random_citation($form_state) {
  module_load_include('inc', 'fedora_repository', 'api/fedora_item');
  module_load_include('inc', 'fedora_repository', 'api/fedora_utils');
  module_load_include('inc', 'fedora_repository', 'CollectionClass');

  $query_string = 'select $object from <#ri> where $object <fedora-model:hasModel> <info:fedora/islandora:citationCModel>';
  $query_string = htmlentities(urlencode($query_string));
  $result = '';
  $url = variable_get('fedora_repository_url', 'http://localhost:8080/fedora/risearch');
  $url .= "?type=tuples&flush=TRUE&format=Sparql&offset=0&lang=itql&stream=on&query=" . $query_string;
  $result .= do_curl($url);
  if ($result == '') {
    drupal_set_message(t('The fedora server is not responding!'));
    return FALSE;
  }

  $result = new SimpleXMLElement($result);

  $entries = $result->results->result;

  foreach ($entries as $entry) {
    $pid = explode('/', $entry->object['uri']);
    $pids[] = $pid[1];
  }

  $rnd = rand(1, count($pids));
  $random_pid = $pids[$rnd];

  $object = new Fedora_Item($random_pid);
  $xmlstr = $object->get_datastream_dissemination('MODS');
  if ($xmlstr) {
    $xml = new SimpleXMLElement($xmlstr);
    $given = array();
    $family = array();
    $name = '';
    $i = 0;
    if (!$xml->name == NULL) {
      foreach ($names = $xml->name as $name) {
        foreach ($name->namePart as $name_part) {
          if ($name_part['type'] == 'given') {
            $citation_names[$i]['given'][] = (string) substr($name_part, 0, 1);
          }
          elseif ($name_part['type'] == 'family') {
            $citation_names[$i]['family'] = (string) $name_part;
          }
        }
        $i++;
      }
      $citation_name_str = '';
      foreach ($citation_names as $citation_name) {
        $citation_name_str .= implode($citation_name['given'], '. ') . '. ' . $citation_name['family'] . ', ';
      }
      $citation_name_str = rtrim($citation_name_str, ', ');
      $citation_info = $xml->titleInfo->title;
      $content = l($citation_info . '; ' . $citation_name_str, 'fedora/repository/' . $random_pid, array('html' => TRUE));
    }
    else {
      $content .= l($citation_info, 'fedora/repository/' . $random_pid, array('html' => TRUE));
    }
  }
  else {
    $content .= l(t('Title not found'), 'fedora/repository/' . $random_pid);
  }

  $form['random_citation'] = array(
    '#value' => $content,
  );

  return $form;
}