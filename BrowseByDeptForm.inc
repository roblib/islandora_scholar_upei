<?php

/**
 * @file
 * 
 */

function scholar_browse_by_dept_form() {
  module_load_include('inc', 'Fedora_Repository', 'api/fedora_utils');
  module_load_include('inc', 'islandora_solr_search', 'IslandoraSolrQueryProcessor');
  $filename = drupal_get_path('module', 'scholar') . '/inclusionList.txt';
  $inclusionlist = file_get_contents($filename);
  $inclusionlist = fix_encoding($inclusionlist);
  $inclusionlist = explode(",", $inclusionlist);
  $ignore_roles_less_than = 0;
  
  $result = file_get_contents('http://localhost:8080/solr/select/?q=rels.hasModel:researcherCModel&fl=mads.topic&rows=500');
  
  $xml = new SimpleXMLElement($result);
  
  $depts = array_unique($xml->xpath('//doc/arr/str'));
  $output = '<ul>';
  foreach ($depts as $dept) {
    if (array_search($dept, $inclusionlist)) {
    $output .= "<li><a href=islandora/solr/search/mads.topic:$dept>" . $dept . '</a></li>';
    }
  }
//  $output = $result;
//  $result = db_query('SELECT role.rid, role.name FROM {role} where role.rid > %d order by role.name', 0);
//  $output .= '<ul>';
//  while ($role = db_fetch_object($result)) {
//
//    if ($role->rid > $ignore_roles_less_than && in_array($role->name, $inclusionlist)) {
//      $urlportion = urlencode($role->name);
//      $name = $role->name;
//      if ('Home Economics' == $name) {
//        $name = "Family and Nutritional Sciences";
//      }
//      else
//      if ('Anatomy Physiology' == $name) {
//        $name = "Biomedical Sciences";
//      }
//      else
//      if ('Womens Studies' == $name) {
//        $name = "Women's Studies";
//      }
//      else
//      if ('Path Micro' == $name) {
//        $name = "Pathology and Microbiology";
//      }
//      else
//      if ('Soc Anth' == $name) {
//        $name = "Sociology and Anthropology";
//      }
//      $output .= "<li><a href=islandora/solr/search/$urlportion>" . $name . '</a></li>';
//    }
//  }
  $output .= '</ul>';

  $form['list'] = array(
    '#value' => $output,
  );
  
  return $form;
}