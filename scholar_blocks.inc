<?php

/*
 * @file
 */

function scholar_recent_items($form_state) {
  module_load_include('inc', 'fedora_repository', 'api/fedora_utils');
  module_load_include('inc', 'fedora_repository', 'api/fedora_item');
  module_load_include('inc', 'Fedora_Repository', 'CollectionClass');
  $query_string = 'select $object $modified from <#ri> where $object <fedora-model:hasModel> <info:fedora/ir:citationCModel> and $object <fedora-view:lastModifiedDate> $modified and $modified <mulgara:after> \'2010-02-22T12:54:59.265Z\'^^<xml-schema:dateTime> in <#xsd> order by $modified desc';
  $query_string = htmlentities(urlencode($query_string));
  $content = '';
  $url = variable_get('fedora_repository_url', 'http://localhost:8080/fedora/risearch');
  $url .= "?type=tuples&flush=TRUE&format=Sparql&limit=20&offset=0&lang=itql&stream=on&query=" . $query_string;
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

  $query = @file_get_contents('http://' . $solr_location . '/select/?q=PID:ir*%20AND%20rels.hasModel:ir\:citationCModel&start=0&rows=0&indent=on');
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

  $results = array_slice($results, 0, 10);

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

  $query_string = 'select $object from <#ri> where $object <fedora-model:hasModel> <info:fedora/ir:citationCModel>';
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

function scholar_frontpage_search($form_state) {
  module_load_include('module', 'islandora_solr_search', 'islandora_solr_search');
  module_load_include('module', 'islandora_solr_search', 'extended_search_form/islandora_extended_search_form');

  $tabset['first_tab'] = array(
    '#type' => 'tabpage',
    '#title' => t('Description'),
  );

  $tabset['first_tab']['tabs'] = array(
    '#type' => 'tabset',
  );

  $tabset['first_tab']['tabs']['simple'] = array(
    '#type' => 'tabpage',
    '#title' => t('Search'),
    '#content' => drupal_get_form('islandora_solr_simple_search_form'),
    '#weight' => 2,
  );

  $tabset['first_tab']['tabs']['advanced'] = array(
    '#type' => 'tabpage',
    '#title' => t('Advanced search'),
    '#content' => drupal_get_form('islandora_extended_search_form', $form_state),
    '#weight' => 2,
  );

  return $tabset;
}