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
    if ($result) {
     
      $output = '<ul>';
      $output .= l(t('Search all authors'), $base_url . '/islandora/solr/search/mods.department_facet:"' . str_replace('~~', ':', $dept_copy) . '"');
  
      foreach($researchers as $res) {
        $output .= l(t('@researcher (@citations)', array('@researcher' => $researcher['full_name'], '@citations' => $researcher['citation_count'])));
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