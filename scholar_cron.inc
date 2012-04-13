<?php

/**
 * @file
 */

/**
 * Cron function to remove the policy datastream from an object if the 
 * datastream has the label 'POLICY' and the mods.embargo_date
 * index field has expired in the last week. If the cron job is set up
 * properly then we sould only ever need to check the last week. Checking
 * further back would increase the number of objects that would need
 * to be checked.
 * Added extra check for a supplemental information embargo.
 */
function scholar_cron_embargo_check() {
  module_load_include('inc', 'fedora_repository', 'api/fedora_item');
  
  $solr_location = variable_get('islandora_solr_search_block_url', 'localhost:8080/solr');

  $document_solr_query = 'http://' . $solr_location . '/select/?q=mods.embargo_date:[NOW-7DAY%20TO%20NOW]%20OR%20-mods.embargo_date:[*%20TO%20*]%20AND%20rels.hasModel:thesisCModel&fl=PID&rows=10000';
  $document_solr_result = @file_get_contents($document_solr_query);

  if ($document_solr_result != NULL) {
    $xml = new SimpleXMLElement($document_solr_result);
    $array = $xml->xpath('//doc');
    $array = array_filter($array);
    $pids = array();
    foreach ($array as $arr) {
      $pid = (string) $arr[0]->str;
      $pids[] = $pid;
      $item = new Fedora_Item($pid);
      $ds_array = $item->get_datastreams_list_as_array();
      foreach ($ds_array as $key => $ds) {
        if ($key == 'POLICY' && in_array($ds['label'], array('Embargo policy - Doc', 'Embargo policy - Supp', 'Embargo policy - Both', 'POLICY'))) {
          $purge_status = $item->purge_datastream('POLICY');
          if ($purge_status != NULL) {
            drupal_set_message(t('Embargo policy removed for @pid', array('@pid' => $pid)));
          }
        }
      }
    }
  }
  
  $supp_solr_query = 'http://' . $solr_location . '/select/?q=mods.supp_embargo_date:[NOW-7DAY%20TO%20NOW]%20OR%20-mods.supp_embargo_date:[*%20TO%20*]%20AND%20rels.hasModel:thesisCModel&fl=PID&rows=10000';
  $supp_solr_result = @file_get_contents($supp_solr_query);
  
  if ($supp_solr_result != NULL) {
    $xml = new SimpleXMLElement($supp_solr_result);
    $array = $xml->xpath('//doc');
    $array = array_filter($array);
    $pids = array();
    foreach ($array as $arr) {
      $pid = (string) $arr[0]->str;
      $pids[] = $pid;
      $item = new Fedora_Item($pid);
      $ds_array = $item->get_datastreams_list_as_array();
      foreach ($ds_array as $key => $ds) {
        if ($key == 'POLICY' && in_array($ds['label'], array('Embargo policy - Doc', 'Embargo policy - Supp', 'Embargo policy - Both', 'POLICY'))) {
          $purge_status = $item->purge_datastream('POLICY');
          if ($purge_status != NULL) {
            drupal_set_message(t('Embargo policy removed for @pid', array('@pid' => $pid)));
          }
        }
      }
    }
  }
}