<?php

/**
 * @file
 */

/**
 * Form to browse the citation objects by department. Each department has its own collapsed fieldset
 * which contains the researchers that are associated with that department. The number of citations per
 * dept and researcher are displayed providing the relevant information is available from the citation
 * object. 
 * @return string 
 */
function scholar_browse_by_dept_form() {
  module_load_include('inc', 'Fedora_Repository', 'api/fedora_utils');
  module_load_include('inc', 'islandora_solr_search', 'IslandoraSolrQueryProcessor');

  $result = file_get_contents('http://localhost:8080/solr/select/?q=rels.hasModel:departmentCModel&fl=mads.identifier&rows=500&sort=mads.identifier_sort+asc');
  $xml = new SimpleXMLElement($result);
  $depts = array_unique($xml->xpath('//doc/arr/str'));

  foreach ($depts as $dept) {
    $output = '';
    $dept_copy = (string) $dept;
    $dept_query = $dept_title = $dept_copy;    
    // Added in mapping for various departments that may be represented by
    // more than one name
    if ($dept_query == 'Pathology and Microbiology') {
      $dept_query = 'Path Micro"%20OR%20mads.topic:"Pathology and Microbiology';
    }
    
    if ($dept_query == 'Political Science') {
      $dept_query = 'Political Studies"%20OR%20mads.topic:"Political Science';
    }
    
    if ($dept_copy == 'Pathology and Microbiology') {
      $dept_copy = 'Path%20Micro"%20OR%20mods.department~~"Pathology%20and%20Microbiology';
    }
    
    if ($dept_copy == 'Department of Biomedical Sciences') {
      $dept_copy = 'Biomedical%20Sciences"%20OR%20mods.department~~"Department%20of%20Biomedical%20Sciences';
    }
    
    if ($dept_copy == 'Applied Human Sciences') {
      $dept_copy = 'Applied%20Human%20Sciences"%20OR%20mods.department~~"Home%20Economics';
    }
    
    if ($dept_copy == 'Biomedical Sciences') {
      $dept_copy = 'Biomedical%20Sciences"%20OR%20mods.department~~"Anatomy%20Physiology';
    }
    
    if ($dept_copy == 'Computer Science and Information Technology') {
      $dept_copy = 'Computer%20Science%20and%20Information%20Technology"%20OR%20mods.department~~"Computer%20Science%20and%20Info%20Technology';
    }
    
    // Replace some of the characters that may be found in the dept names so that they don't break
    // the solr query
    $dept_result_query = 'http://localhost:8080/solr/select/?q=mods.department:"' . (str_replace(array(' ', ":", '&', '~~'), array('%20', '?', '?', ':'), ($dept_copy))) . '"';
    $dept_result = file_get_contents($dept_result_query);
    $dept_xml = new SimpleXMLElement($dept_result);
    $number_of_citations = $dept_xml->xpath('./result[@name="response"]/@numFound');
    $citations = " (" . (string) $number_of_citations[0] . ")";
    $dept_link = str_replace(' ', '%20', $dept_copy);

    $form[$dept_title] = array(
      '#type' => 'fieldset',
      '#title' => t('@dept_title @citations', array('@dept_title' => $dept_title, '@citations' => $citations)),
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
    );

    $dept_copy = str_replace(array(' ', '&', ':'), array('%20', '%26', '%3A'), $dept_copy);
    $dept_query = str_replace(array(' ', '&', ':'), array('%20', '%26', '%3A'), $dept_query);
    $query = 'http://localhost:8080/solr/select/?q=mads.topic_facet:"' . $dept_query . '"&fl=mads.family,mads.given,mads.fullname,mads.username&rows=10000&sort=mads.family_sort+asc';
    $result = file_get_contents($query);
    $xml = new SimpleXMLElement($result);
    $array = $xml->xpath('//doc');
    $output = '<ul>';
    $output .= l(t('Search all authors'), 'islandora/solr/search/mods.department:"' . str_replace('~~', ':', $dept_copy) . '"');
    $users = array();
    $user = '';
    $key = '';
    $link = array();
    $author_citations = array();
    $array = array_filter($array);
    foreach ($array as $arr) {
      $family = $arr->xpath('./arr[@name="mads.family"]/str');
      $given = $arr->xpath('./arr[@name="mads.given"]/str');
      $fullname = $arr->xpath('./arr[@name="mads.fullname"]/str');
      $username = $arr->xpath('./arr[@name="mads.username"]/str');
      if ($username[0] != '') {
        $username_query = file_get_contents('http://localhost:8080/solr/select/?q=mods.username:' . (str_replace(';', ' ', (strtolower($username[0])))));
        $username_xml = new SimpleXMLElement($username_query);
        $number_of_citations = $username_xml->xpath('./result[@name="response"]/@numFound');
        $author_citations[] = $number_of_citations[0];
      }
      $users[] = $family[0] . ', ' . $given[0];
      $link[] = '<a href=/islandora/solr/search/mods.username:"' . $username[0] . '">';
    }
    asort($users);
    foreach ($users as $key => $user) {
      if ($author_citations[$key] > 0) {
        $output .= '<li>' . $link[$key] . $user . '</a><i> (' . (string) $author_citations[$key] . ')</i></li>';
      }
    }
    $output .= '</ul>';

    $form[$dept_title]['authors'] = array(
      '#value' => $output,
      '#prefix' => '<div>',
      '#suffix' => '</div>',
    );
  }

//  var_dump($form);
  return $form;
}