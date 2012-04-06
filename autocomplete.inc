<?php

/**
 * @file
 */


/**
 *
 * @param type $content_model
 * @param type $index_field
 * @param type $string
 * @return type 
 */
function islandora_autocomplete($content_model = '', $index_field = '', $string = '') {
  if ($content_model == '' || $index_field == '') {
    $output[''] = '';
    return drupal_json($output);
  }
  $string = str_replace('*', '', $string);
  if ($string == '') {
    $results = islandora_scholar_get_values($content_model, $index_field);
  }
  else {
    $query = $content_model . '%20AND%20' . $index_field . ':' . strtolower($string) . '*';
    $results = islandora_scholar_get_values($query, $index_field);
  }
  if ($results == 'None') {
    $output[''] = '';
    return drupal_json($output);
  }
  else {
    array_filter($results);
    foreach ($results as $result) {
      $output[$result] = $result;
    }
  }
  return drupal_json($output);
}

/**
 *
 * @param type $query Criteria to search for as a solr query
 * @param type $results The field that you want returned
 * @return type An array of results with the key and element being the same 
 */
function islandora_scholar_get_values($query, $results) {

  $solr_url = variable_get('fedora_solr_search_url', 'http://localhost:8080/solr');

  if (is_array($results)) {
    $result_string = implode(',', $results);
    $query = file_get_contents($solr_url . '/select?q=' . $query . '&fl=' . $result_string);
  }
  else {
    $query = file_get_contents($solr_url . '/select?q=' . $query . '&fl=' . $results);
  }
  $result = new SimpleXMLElement($query);
  $number = $result->xpath('./result[@name="response"]/@numFound');
  if ((int) $number[0]['numFound'] < 1) {
    return 'None';
  }
  $researchers = $result->result;
  $list = array();
  foreach ($researchers->children() as $researcher) {
    $values = $researcher->arr[0]->str;
    foreach ($values as $value) {
      $value = (string) $value;
      $list[$value] = $value;
    }
  }
  return $list;
}