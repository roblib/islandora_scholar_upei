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
  $url .= "?type=tuples&flush=TRUE&format=Sparql&limit=100&offset=0&lang=itql&stream=on&query=" . $query_string;
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
  $refworks = $object->get_datastream_dissemination('refworks');
//  drupal_set_message('Input: ' . $refworks);
  $input = new DOMDocument();
  $input->loadXML($refworks);
  $xslt = new XSLTProcessor();
  $xsl = new DOMDocument();
  $xslt_file = '/var/www/drupal/sites/all/modules/islandora_scholar_upei/xsl/refworks_to_mods.xsl';
  $xsl->load($xslt_file);
  $xslt->importStylesheet($xsl);
  $output = $xslt->transformToDoc($input);
//  var_dump($output);
  $xml = (string)$output->saveXML();
//  $xml = 'This is a test';
  $object->add_datastream_from_string($xml, 'MODS', 'MODS datastream', 'text/xml', 'X');
//  drupal_set_message('Output: ' . $xml);
  }
}