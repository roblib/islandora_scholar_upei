<?php

/*
 * @file
 */

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
