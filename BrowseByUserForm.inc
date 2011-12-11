<?php

/**
 * @file
 *
 * Contains the hooks for rendering and processing the Browse By User form.
 */

/**
 * Builds a drupal form for launching a browse by user search.
 * 
 * @param array $form_state 
 * 
 * @return array
 */
function scholar_browse_by_user_form(array &$form_state, $initial = 'A') {
  drupal_add_css('scholar.css');
  $researchers = array();
  $letters = array('A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z');
  $list = '';
  foreach ($letters as $letter) {
    if ($letter == 'Z') {
      $list .= l($letter, 'browse_users/' . $letter);
    }
    else {
      $list .= l($letter, 'browse_users/' . $letter) . ', ';
    }
  }

  $form['choice'] = array(
    '#value' => l('By name', 'browse_users/A') . ' | ' . l('By department', 'browse_users_dept/Robertson%20Library') . "</br></br>",
  );

  $form['list'] = array(
    '#value' => $list,
  );
  $result = file_get_contents('http://localhost:8080/solr/select/?q=mads.family:' . strtolower($initial) . '*&fl=mads.family,mads.given&rows=10000&sort=mads.family_sort+asc');
  $xml = new SimpleXMLElement($result);
  $array = $xml->xpath('//doc');
  $output = '<ul>';
  $users = array();
  $array = array_filter($array);
  foreach ($array as $arr) {
    $family = $arr->xpath('./arr[@name="mads.family"]/str');
    $given = $arr->xpath('./arr[@name="mads.given"]/str');
    $users[] = $family[0] . ', ' . $given[0];
    $link[] = '<li><a href=/islandora/solr/search/"' . $given[0] . '%20' . $family[0] . '">';
  }
  asort($users);
  foreach ($users as $key => $user) {
    $output .= $link[$key] . $user . '</a></li>';
  }

  $output .= '</ul>';

  $form['author'] = array(
    '#value' => $output,
  );

  return $form;
}

function scholar_browse_by_user_dept_form(&$form_state, $dept = 'Robertson Library') {
  module_load_include('inc', 'Fedora_Repository', 'api/fedora_utils');

  $filename = drupal_get_path('module', 'scholar') . '/inclusionList.txt';
  $inclusionlist = file_get_contents($filename);
  $inclusionlist = fix_encoding($inclusionlist);
  $inclusionlist = explode(",", $inclusionlist);
  $inclusionlist = array_filter($inclusionlist);
  asort($inclusionlist);

  $list = '';
  
  foreach ($inclusionlist as $entry) {
    $list .= l($entry, 'browse_users_dept/' . $entry) . ' | ';
  }
  
  $form['choice'] = array(
    '#value' => l('By name', 'browse_users/A') . ' | ' . l('By department', 'browse_users_dept/Robertson%20Library') . "</br></br>",
  );
  
  $form['list'] = array(
    '#value' => $list,
  );
  $dept = str_replace(' ', '%20', $dept);
  $query = 'http://localhost:8080/solr/select/?q=mads.topic:' . $dept . '&fl=mads.family,mads.given&rows=10000&sort=mads.family_sort+asc';
  $result = file_get_contents($query);
  $xml = new SimpleXMLElement($result);
  $array = $xml->xpath('//doc');
  $output = '<ul>';
  $users = array();
  $array = array_filter($array);
  foreach ($array as $arr) {
    $family = $arr->xpath('./arr[@name="mads.family"]/str');
    $given = $arr->xpath('./arr[@name="mads.given"]/str');
    $users[] = $family[0] . ', ' . $given[0];
    $link[] = '<li><a href=/islandora/solr/search/"' . $given[0] . '%20' . $family[0] . '">';
  }
  asort($users);
  foreach ($users as $key => $user) {
    $output .= $link[$key] . $user . '</a></li>';
  }

  $output .= '</ul>';

  $form['author'] = array(
    '#value' => $output,
  );
  
  return $form;
}