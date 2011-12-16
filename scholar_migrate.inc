<?php

/**
 * @file
 * 
 */
function scholar_migrate_refworks_form($form) {
  module_load_include('inc', 'fedora_repository', 'api/fedora_utils');

  $query_string = 'select $object from <#ri> where ($object <fedora-model:hasModel> <info:fedora/islandora:refworksCModel> and $object <fedora-model:state> <info:fedora/fedora-system:def/model#Active>)';
    $query_string = htmlentities(urlencode($query_string));
    $content = '';
    $url = variable_get('fedora_repository_url', 'http://localhost:8080/fedora/risearch');
    $url .= "?type=tuples&flush=TRUE&format=Sparql&limit=10&offset=0&lang=itql&stream=on&query=" . $query_string;
    $content .= do_curl($url);
    $result = new SimpleXMLElement($content);
  
    var_dump($result);

    $form['list'] = array(
      '#value' => t('Test'),
    );
    
  return $form;
}