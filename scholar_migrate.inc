<?php

/**
 * @file
 * 
 */
function scholar_migrate_refworks_table() {
  module_load_include('inc', 'fedora_repository', 'api/fedora_utils');

  $query_string = 'select $object $title from <#ri> where ($object <fedora-model:hasModel> <info:fedora/islandora:refworksCModel> and $object <dc:title> $title and $object <fedora-model:state> <info:fedora/fedora-system:def/model#Active>)';
  $query_string = htmlentities(urlencode($query_string));
  $content = '';
  $url = variable_get('fedora_repository_url', 'http://localhost:8080/fedora/risearch');
  $url .= "?type=tuples&flush=TRUE&format=Sparql&limit=200&offset=0&lang=itql&stream=on&query=" . $query_string;
  $content .= do_curl($url);
  $query = new SimpleXMLElement($content);
  $list = '';
  $results = $query->results->result;
  foreach ($results as $result) {
    $pid = explode('/', $result->object['uri']);
    $title = $result->title;
    $objects[$pid[1]] = $title;
    $keys[] = $pid[1];
  }

  $form['list'] = array(
    '#value' => $list,
  );

  $table = array(
    '#header' => array(theme('table_select_header_cell'), t('PID'), t('Title'), ''),
    '#theme' => THEME_BIBLIOGRAPHY_FORM_TABLE,
    '#tree' => TRUE,
    'rows' => array(),
    'selections' => array(
      '#type' => 'checkboxes',
      '#options' => array_fill_keys($keys, ''),
    ),
  );
  $rows = &$table['rows'];
  foreach ($objects as $key => $object) {
    $rows[] = array(
      '#pid' => $key,
      'pid' => array('#value' => l($key, 'fedora/repository/' . $key)),
      'title' => array('#value' => $object),
    );
  }

  return $table;
}

function scholar_migrate_refworks_form() {

  $table = scholar_migrate_refworks_table();

  $form['table'] = array(
    'table' => $table,
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Submit'),
  );
  return $form;
}

function scholar_migrate_refworks_form_submit($form, &$form_state) {
  module_load_include('inc', 'fedora_repository', 'api/fedora_item');
  $pids = array_filter($form_state['values']['table']['selections']);
  drupal_set_message(implode(', ', $pids));
  foreach ($pids as $pid) {
    $object = new Fedora_Item($pid);
//  var_dump($object);
    $datastreams = $object->get_datastreams_list_as_array();
    if (!array_key_exists('MODS', $datastreams)) {
      $refworks = $object->get_datastream_dissemination('refworks');
//  drupal_set_message('Input: ' . $refworks);
      $input = new DOMDocument();
      $input->loadXML($refworks);
      $xslt = new XSLTProcessor();
      $xsl = new DOMDocument();
      $xslt_file = drupal_get_path('module', 'scholar') . '/xsl/refworks_to_mods.xsl';
      $xsl->load($xslt_file);
      $xslt->importStylesheet($xsl);
      $output = $xslt->transformToDoc($input);
      $xml = (string) $output->saveXML();
      $u1 = str_replace(' ', '_', $input->getElementsByTagName('u1')->item(0)->nodeValue);
      $u2 = str_replace(' ', '_', $input->getElementsByTagName('u2')->item(0)->nodeValue);
      $object->add_datastream_from_string($xml, 'MODS', 'MODS datastream', 'text/xml', 'X');

//      $rel0 = $object->get_relationships('hasModel');
//      if ($rel0['object'] == 'islandora:refworksCModel') {
//      }
//  drupal_set_message('u1: ' . $u1);
//  drupal_set_message('u2: ' . $u2);
      // Check if the relationships for u1 and u2 exist
      // If they don't they create them in the RELS-EXT
      // TODO: create exist_relationship function to use elsewhere
      $rel1 = $object->get_relationships('hasResearcher');
      if ($rel1 == NULL) {
        $object->add_relationship('hasResearcher', $u1);
      }
      else {
        drupal_set_message(t('Relationship hasResearcher already exists!'), 'warning');
      }
      $rel2 = $object->get_relationships('hasDepartment');
      if ($rel2 == NULL) {
        $object->add_relationship('hasDepartment', $u2);
      }
      else {
        drupal_set_message(t('Relationship hasDepartment already exists!'), 'warning');
      }
    }
    else {
      drupal_set_message(t('MODS datastream already exists for @pid', array('@pid' => $pid)), 'warning');
    }
    $model_test = $object->get_models();
    if ($model_test[0]['object'] == 'islandora:refworksCModel') {
      $model_result = $object->purge_relationship('hasModel', 'islandora:refworksCModel');
      if ($model_result) {
        $object->add_relationship('hasModel', 'ir:citationCModel', FEDORA_MODEL_URI);
      }
    }
    $collection_test = $object->get_relationships('fedora:isMemberOfCollection');
    if ($collection_test[0]['object'] == 'ir:collection') {
      $collection_result = $object->purge_relationship('fedora:isMemberOfCollection', 'ir:collection');
      if ($collection_result) {
        $object->add_relationship('fedora:isMemberOfCollection', 'ir:citationCollection');
      }
    }
  }
}

//function islandora_scholar_check_relationship($pid, $relationship) {
//    module_load_include('inc', 'fedora_repository', 'api/fedora_item');
//  $object = new Fedora_Item($pid);
//  $test = $object->get_relationships($relationship);
//  if ($test == NULL) {
//    return FALSE;
//  }
//  else {
//    return TRUE;
//  }
//}