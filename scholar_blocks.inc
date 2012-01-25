<?php

/*
 * @file
 */

function scholar_recent_items($form_state) {
  module_load_include('inc', 'fedora_repository', 'api/fedora_utils');
  module_load_include('inc', 'Fedora_Repository', 'CollectionClass');
  $query_string = 'select $object $modified from <#ri> where $object <fedora-model:hasModel> <info:fedora/ir:citationCModel> and $object <fedora-view:lastModifiedDate> $modified and $modified <mulgara:after> \'2010-02-22T12:54:59.265Z\'^^<xml-schema:dateTime> in <#xsd> order by $modified desc';
  $query_string = htmlentities(urlencode($query_string));
  $content = '';
  $url = variable_get('fedora_repository_url', 'http://localhost:8080/fedora/risearch');
  $url .= "?type=tuples&flush=TRUE&format=Sparql&limit=10&offset=0&lang=itql&stream=on&query=" . $query_string;
  $content .= do_curl($url);
  if ($content == '') {
    drupal_set_message(t('The fedora server is not responding!'));
    return FALSE;
  }
  $result = new SimpleXMLElement($content);

//    var_dump($result);
  $entries = $result->results->result;

  $list = '<ul>';

  foreach ($entries as $entry) {
    $pid = explode('/', $entry->object['uri']);
    $collection_helper = new CollectionClass();
    $xmlstr = $collection_helper->getStream($pid[1], "MODS");
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

//  var_dump($content);

  $form['title'] = array(
    '#value' => $list,
  );

  return $form;
}

function scholar_total_records($form_state) {
  $query = file_get_contents('http://localhost:8080/solr/select/?q=PID:ir*');
  $xml = new SimpleXMLElement($query);
  $number_of_records = $xml->xpath('./result[@name="response"]/@numFound');

  $form['records'] = array(
    '#value' => t('@records records added', array('@records' => $number_of_records[0])),
  );

  return $form;
}

function scholar_total_text_downloads($form_state) {
  $query = db_query("select count(pid) from {islandora_scholar_ds_tracking} where dsid = 'PDF' OR dsid = 'OBJ';");
  $count = db_result($query);

  $form['full-text_downloads'] = array(
    '#value' => t('@downloads full-text downloads', array('@downloads' => $count)),
  );

  return $form;
}

function scholar_total_citation_views($form_state) {
  $query = db_query("select count(pid) from {islandora_scholar_page_tracking}");
  $count = db_result($query);

  $form['citation_views'] = array(
    '#value' => t('@views citation views', array('@views' => $count)),
  );

  return $form;
}

function scholar_top_downloads($form_state) {
  
}

function scholar_top_viewed($form_state) {
  module_load_include('inc', 'fedora_repository', 'CollectionClass');
  $query = db_query("select pid, count(*) from {islandora_scholar_page_tracking} group by pid order by count(*) DESC");

  while (db_fetch_array($query)) {
    $results[] = db_fetch_array($query);
  }

  $results = array_slice($results, 0, 10);

  $content = '<ul>';
  foreach ($results as $result) {
    $pid = $result['pid'];
    $collection_helper = new CollectionClass();
    $xmlstr = $collection_helper->getStream($pid, "MODS");
    if ($xmlstr) {
      $xml = new SimpleXMLElement($xmlstr);
      $content .= '<li>' . l($xml->titleInfo->title, 'fedora/repository/' . $pid, array('html' => TRUE)) . '</li>';
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
