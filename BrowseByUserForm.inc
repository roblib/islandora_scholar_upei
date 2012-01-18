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
      $list .= l($letter, 'browse_authors/' . $letter);
    }
    else {
      $list .= l($letter, 'browse_authors/' . $letter) . ', ';
    }
  }

  $form['choice'] = array(
    '#value' => l('By name', 'browse_authors/A') . ' | ' . l('By department', 'browse_users_dept/Robertson%20Library') . "</br></br>",
  );

  $form['list'] = array(
    '#value' => $list,
  );
  $result1 = file_get_contents('http://localhost:8080/solr/select/?q=mods.username:' . strtolower($initial) . '*&fl=mods.username&rows=10000');
  $xml1 = new SimpleXMLElement($result1);
  $array1 = array_unique(array_filter($xml1->xpath('//doc//str')));
  var_dump($array1);
  foreach ($array1 as $arr1) {
  $result = file_get_contents('http://localhost:8080/solr/select/?q=mads.username:' . strtolower($arr1) . '&fl=mads.family,mads.given,mads.username&rows=10000&sort=mads.family_sort+asc');
  $xml = new SimpleXMLElement($result);
  $array = $xml->xpath('//doc');
  $output = '<ul>';
  $users = array();
  $array = array_filter($array);
  var_dump($result);
    $family = $array->xpath('./arr[@name="mads.family"]/str');
    $given = $array->xpath('./arr[@name="mads.given"]/str');
    $username = $array->xpath('./arr[@name="mads.username"]/str');
    if ($username[0] != '') {
      $username_query = file_get_contents('http://localhost:8080/solr/select/?q=mods.username:' . (str_replace(';', ' ', (strtolower($username[0])))));
      $username_xml = new SimpleXMLElement($username_query);
      $number_of_citations = $username_xml->xpath('./result[@name="response"]/@numFound');
        $citations[] = " (" . (string) $number_of_citations[0] . ")";
    }
    $users[] = $family[0] . ', ' . $given[0];
    $link[] = '<li><a href=/islandora/solr/search/mods.username:"' . $username[0] . '">';
  }
  asort($users);
  foreach ($users as $key => $user) {
    $output .= $link[$key] . $user . '</a><i>' . $citations[$key] . '</i></li>';
  }

  $output .= '</ul>';

  $form['author'] = array(
    '#value' => $output,
  );

  return $form;
}

function scholar_browse_by_user_dept_form(&$form_state, $dept = 'Robertson Library') {
  module_load_include('inc', 'Fedora_Repository', 'api/fedora_utils');

  $result = file_get_contents('http://localhost:8080/solr/select/?q=rels.hasModel:departmentCModel&fl=mads.identifier&rows=500&sort=mads.identifier_sort+asc');

  $xml = new SimpleXMLElement($result);

  $inclusionlist = array_unique($xml->xpath('//doc/arr/str'));

  $list = '';

  foreach ($inclusionlist as $entry) {
    $list .= l($entry, 'browse_users_dept/' . $entry) . ' | ';
  }

  $form['choice'] = array(
    '#value' => l('By name', 'browse_authors/A') . ' | ' . l('By department', 'browse_users_dept/Robertson%20Library') . "</br></br>",
  );

  $form['list'] = array(
    '#value' => $list,
  );
  $dept = str_replace(' ', '%20', $dept);
  $query = 'http://localhost:8080/solr/select/?q=mads.topic:' . $dept . '&fl=mads.family,mads.given,mads.fullname,mads.username&rows=10000&sort=mads.family_sort+asc';
  $result = file_get_contents($query);
  $xml = new SimpleXMLElement($result);
  $array = $xml->xpath('//doc');
  $output = '<ul>';
  $users = array();
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
        $citations[] = " (" . (string) $number_of_citations[0] . ")";
    }
    $users[] = $family[0] . ', ' . $given[0];
    $link[] = '<li><a href=/islandora/solr/search/mods.username:"' . $username[0] . '">';
  }
  asort($users);
  foreach ($users as $key => $user) {
    $output .= $link[$key] . $user . '</a><i>' . $citations[$key] . '</i></li>';
  }

  $output .= '</ul>';

  $form['author'] = array(
    '#value' => $output,
  );

  return $form;
}