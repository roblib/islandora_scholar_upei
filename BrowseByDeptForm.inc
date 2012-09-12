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

  $query = db_query("SELECT pid,full_name,citation_count from {islandora_scholar_dept_citation_counts} WHERE citation_count > 0 ORDER BY full_name ASC");

  while ($dept = db_fetch_array($query)) {
    $depts[] = $dept;
  }

  foreach ($depts as $dept) {
    $output = '';
    $dept_title = $dept['full_name'];
    $form[$dept_title] = array(
      '#type' => 'fieldset',
      '#title' => t('@dept_title (@citations)', array('@dept_title' => $dept_title, '@citations' => $dept['citation_count'])),
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
    );

    $dept_query = db_query("SELECT pid,full_name,citation_count from {islandora_scholar_researcher_citation_counts} WHERE department = '%s' ORDER BY full_name ASC", $dept_title);

    $dept_copy = $dept_title;

    if ($dept_copy == 'Pathology and Microbiology') {
      $dept_copy = 'Path Micro" OR mods.department_facet~~"Pathology and Microbiology';
    }

    if ($dept_copy == 'School of Business') {
      $dept_copy = 'School of Business" OR mods.department_facet~~"Business';
    }

    if ($dept_copy == 'Political Science') {
      $dept_copy = 'Political Studies" OR mods.department_facet~~"Political Science';
    }

    if ($dept_copy == 'Sociology and Anthropology') {
      $dept_copy = 'Sociology and Anthropology" OR mods.department_facet~~"Soc Anth';
    }

    if ($dept_copy == 'Women\'s Studies') {
      $dept_copy = 'Women\'s Studies" OR mods.department_facet~~"Womens Studies';
    }

    if ($dept_copy == 'Department of Biomedical Sciences') {
      $dept_copy = 'Biomedical Sciences" OR mods.department_facet~~"Department of Biomedical Sciences';
    }

    if ($dept_copy == 'Applied Human Sciences') {
      $dept_copy = 'Applied Human Sciences" OR mods.department_facet~~"Home Economics';
    }

    if ($dept_copy == 'Biomedical Sciences') {
      $dept_copy = 'Biomedical Sciences" OR mods.department_facet~~"Anatomy Physiology';
    }

    if ($dept_copy == 'Computer Science and Information Technology') {
      $dept_copy = 'Computer Science and Information Technology" OR mods.department_facet~~"Computer Science and Info Technology';
    }

    $researchers = array();

    while ($researcher = db_fetch_array($dept_query)) {
      $researchers[] = $researcher;
    }
//    var_dump($researchers);
    if ($researchers) {
      $output = '<ul>';
      $output .= l(t('Search all authors'), $base_url . '/islandora/solr/search/mods.department_facet:"' . (str_replace(array(';', ' ', '&', '~~'), array(' ', '%20', '?', ':'), $dept_copy)) . '"');
      foreach ($researchers as $res) {
        if ($res['citation_count'] > 0) {
          $output .= '</br>';
          $output .= l(t('@researcher (@citations)', array('@researcher' => $res['full_name'], '@citations' => $res['citation_count'])), $base_url . '/islandora/solr/search/mods.username:' . $res['pid']);
        }
      }
      $output .= '</ul>';
    }
    else {
      $output = '<ul>';
      $output .= l(t('Search all authors'), $base_url . '/islandora/solr/search/mods.department_facet:"' . (str_replace(array(';', ' ', '&', '~~'), array(' ', '%20', '?', ':'), $dept_copy)) . '"');
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