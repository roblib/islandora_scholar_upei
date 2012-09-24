<?php

/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

  /**
 * runs and itql query and returns the results in an array, array is formatted for form api select element
 * @param string $query
 * @return array
 */
function islandora_scholar_query($query, $use_value_as_key = FALSE) {
  module_load_include('inc', 'fedora_repository', 'CollectionClass');
  $collection = new CollectionClass();
  $results = $collection->getRelatedObjects(NULL, 1000000, NULL, $query);
  $results = trim($results);
  $ret = array();
  if ($results != '') {
    $document = new DOMDocument();
    $document->loadXML($results);
    $path = new DOMXPath($document);
    $path->registerNamespace('sparql', 'http://www.w3.org/2001/sw/DataAccess/rf1/result');
    $results = $path->query('//sparql:result');
    $count = $results->length;
    if ($count > 0) {
      for ($i = 0; $i < $count; $i++) {
        $result = $results->item($i);
        $pid = substr($path->query('sparql:object/@uri', $result)->item(0)->value, 12); // 12 characteres in "info:fedora/"
        $label = $path->query('sparql:title', $result)->item(0)->textContent;
        if (strlen($label) > 125) {
          $label = substr($label, 0, 125); //this text will be shown in a form select so keep it short
        }
        if (!empty($label)) {
          if($use_value_as_key){
            $ret[$label] = $label;
          } else {
            $ret[$pid] = $label;
          }
        }
      }
    }
    return $ret;
  }
  return $ret;
}
?>