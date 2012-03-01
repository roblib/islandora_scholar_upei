<?php

/**
 * @file
 */

function scholar_cron_embargo_check() {
  $solr_query = 'http://localhost:8080/solr/select/?q=mods.embargo_date:[NOW-7DAY%20TO%20NOW]&fl=PID&rows=10000';
  $solr_result = @file_get_contents($solr_query);
  
  if ($solr_result != NULL) {
    $xml = new SimpleXMLElement($solr_result);
  }
}