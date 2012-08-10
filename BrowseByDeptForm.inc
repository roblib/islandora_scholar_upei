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
  
  global $base_url;
  
  module_load_include('inc', 'Fedora_Repository', 'api/fedora_utils');

  $form = array();
  $depts = array();
  
  $query = db_query("SELECT pid,full_name,citation_count from {islandora_scholar_dept_citation_counts}");
  
  while ($dept = db_fetch_array($query)) {
    $depts[] = $dept;
  }
  
  foreach ($depts as $dept) {
    $output = '';
    $dept_title = $dept['full_name'];
    $form[$dept_title] = array(
      '#type' => 'fieldset',
      '#title' => t('@dept_title @citations', array('@dept_title' => $dept_title, '@citations' => $dept['citation_count'])),
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
    );
    
    $dept_query = db_query("SELECT pid,full_name,citation_count from {islandora_scholar_researcher_citation_counts WHERE department = '%s'", $dept);
    
    $researchers = array();
    
    while ($researcher = db_fetch_array($dept_query)) {
      $researchers[] = $researcher;
    }
    var_dump($researchers);
    
    $query = 'http://' . $solr_location . '/select/?q=mads.organization:"' . $dept_query . '"&fl=mads.family,mads.given,mads.fullname,mads.username&rows=10000&sort=mads.family_sort+asc';
    $result = @file_get_contents($query);
//    var_dump($query);
    if ($result) {
      $xml = new SimpleXMLElement($result);
      $array = $xml->xpath('//doc');
      $output = '<ul>';
      $output .= l(t('Search all authors'), $base_url . '/islandora/solr/search/mods.department_facet:"' . str_replace('~~', ':', $dept_copy) . '"');
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
          $username_query = file_get_contents('http://' . $solr_location . '/select/?q=mods.username:' . (str_replace(';', ' ', (strtolower($username[0])))));
          $username_xml = new SimpleXMLElement($username_query);
          $number_of_citations = $username_xml->xpath('./result[@name="response"]/@numFound');
          $author_citations[] = $number_of_citations[0];
        }
        $users[] = $family[0] . ', ' . $given[0];
        $link[] = '<a href=' . $base_url . '/islandora/solr/search/mods.username:"' . $username[0] . '">';
      }
      asort($users);
      foreach ($users as $key => $user) {
        if ($author_citations[$key] > 0) {
          $output .= '<li>' . $link[$key] . $user . '</a><i> (' . (string) $author_citations[$key] . ')</i></li>';
        }
      }
      $output .= '</ul>';
    }
    else {
      $output = '<ul>';
      $output .= l(t('Search all authors'), $base_url . '/islandora/solr/search/mods.department_facet:"' . str_replace('~~', ':', $dept_copy) . '"');
      $output .= '</ul>';
    }

    $form[$dept_title]['authors'] = array(
      '#value' => $output,
      '#prefix' => '<div>',
      '#suffix' => '</div>',
    );
  }

  return $form;
}