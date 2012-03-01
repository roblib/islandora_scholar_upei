<?php

/**
 * @file
 */
function scholar_cron_embargo_check() {
  module_load_include('inc', 'fedora_repository', 'api/fedora_item');
  $solr_query = 'http://localhost:8080/solr/select/?q=mods.embargo_date:[NOW-7DAY%20TO%20NOW]&fl=PID&rows=10000';
  $solr_result = @file_get_contents($solr_query);

  if ($solr_result != NULL) {
    $xml = new SimpleXMLElement($solr_result);
    $array = $xml->xpath('//doc');
    $array = array_filter($array);
    $pids = array();
    foreach ($array as $arr) {
      $pid = (string) $arr[0]->str;
      $pids[] = $pid;
      $item = new Fedora_Item($pid);
      $ds_array = $item->get_datastreams_list_as_array();
      foreach ($ds_array as $key => $ds) {
        drupal_set_message('KEY: ' . $key);
      drupal_set_message('DS: ' . implode(', ', $ds));
        
      }
    }
    drupal_set_message('PIDS: ' . implode(', ', $pids));
  }
}