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
  drupal_add_css('css/scholar.css');

  $solr_location = variable_get('islandora_solr_search_block_url', 'localhost:8080/solr');

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

//  $form['choice'] = array(
//    '#value' => l('By name', 'browse_authors/A') . ' | ' . l('By department', 'browse_users_dept/Robertson%20Library') . "</br></br>",
//  );

  $form['list'] = array(
    '#value' => $list,
  );
  $result = file_get_contents('http://' . $solr_location . '/select/?q=mads.family:' . strtolower($initial) . '*&fl=mads.family,mads.given,mads.username&rows=10000&sort=mads.family_sort+asc');
  $xml = new SimpleXMLElement($result);
  $array = $xml->xpath('//doc');
  $output = '<ul>';
  $users = array();
  $array = array_filter($array);
  foreach ($array as $arr) {
    $family = $arr->xpath('./arr[@name="mads.family"]/str');
    $given = $arr->xpath('./arr[@name="mads.given"]/str');
    $username = $arr->xpath('./arr[@name="mads.username"]/str');
    if ($username[0] != '') {
      $username_str = 'http://' . $solr_location . '/select/?q=mods.username:' . (str_replace(';', ' ', (strtolower($username[0]))));
      $username_query = file_get_contents($username_str);
      $username_xml = new SimpleXMLElement($username_query);
      $number_of_citations = $username_xml->xpath('./result[@name="response"]/@numFound');
      $citations[] = $number_of_citations[0];
    }
    $users[] = $family[0] . ', ' . $given[0];
    $link[] = '<li><a href=/islandora/solr/search/mods.username:"' . $username[0] . '">';
  }
  asort($users);
  foreach ($users as $key => $user) {
    if ($citations[$key] > 0) {
      $output .= $link[$key] . $user . '</a><i> (' . (string) $citations[$key] . ')</i></li>';
    }
  }

  $output .= '</ul>';

  $form['author'] = array(
    '#value' => $output,
  );

  return $form;
}

function scholar_browse_by_user_dept_form(&$form_state, $dept = 'Robertson Library') {
  module_load_include('inc', 'Fedora_Repository', 'api/fedora_utils');

  $solr_location = variable_get('islandora_solr_search_block_url', 'localhost:8080/solr');

  $result = file_get_contents('http://' . $solr_location . '/select/?q=rels.hasModel:departmentCModel&fl=mads.identifier&rows=500&sort=mads.identifier_sort+asc');

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
  $query = 'http://' . $solr_location . '/?q=mads.topic:' . $dept . '&fl=mads.family,mads.given,mads.fullname,mads.username&rows=10000&sort=mads.family_sort+asc';
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
      $username_query = file_get_contents('http://' . $solr_location . '/select/?q=mods.username:' . (str_replace(';', ' ', (strtolower($username[0])))));
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

function islandora_scholar_browse_by_author_form(&$form_state, $search = NULL) {
  $solr_location = variable_get('islandora_solr_search_block_url', 'localhost:8080/solr');

  if ($search == NULL) {
    $result = file_get_contents('http://' . $solr_location . '/select/?q=rels.hasModel:researcher&fl=mads.family,mads.given,mads.username,mads.organization,mads.email,mads.phone,mads.fieldOfActivity&rows=10000&sort=mads.family_sort+asc');    
  }
  else {
    $result = file_get_contents('http://' . $solr_location . '/select/?q=mads.given:' . $search . '%20OR%20mads.family:' . $search . '%20OR%20mads.organization:' . $search . '&fq=rels.hasModel:researcher&fl=mads.family,mads.given,mads.username,mads.organization,mads.email,mads.phone,mads.fieldOfActivity&rows=10000&sort=mads.family_sort+asc');
  }

  $xml = new SimpleXMLElement($result);
  $array = $xml->xpath('//doc');
  $users = array();
  $array = array_filter($array);
  foreach ($array as $arr) {
    $family = $arr->xpath('./arr[@name="mads.family"]/str');
    $given = $arr->xpath('./arr[@name="mads.given"]/str');
    $username = $arr->xpath('./arr[@name="mads.username"]/str');
    $department = $arr->xpath('./arr[@name="mads.organization"]/str');
    $department_link = strtolower(str_replace(' ', '_', $department[0]));
    $phone_number = $arr->xpath('./arr[@name="mads.phone"]/str');
    $email = $arr->xpath('./arr[@name="mads.email"]/str');
    $field_of_activity = $arr->xpath('./arr[@name="mads.fieldOfActivity"]/str');
    if ($username[0] != '') {
      $username_str = 'http://' . $solr_location . '/select/?q=mods.username:' . (str_replace(';', ' ', (strtolower($username[0]))));
      $username_query = file_get_contents($username_str);
      $username_xml = new SimpleXMLElement($username_query);
      $number_of_citations = $username_xml->xpath('./result[@name="response"]/@numFound');
      $citations = $number_of_citations[0];
    }
    $users[] = array(l($given[0] . ' ' . $family[0], 'fedora/repository/ir:' . strtolower($username[0])), l($department[0], 'fedora/repository/ir:' . $department_link), $email[0], $phone_number[0], theme('item_list', $field_of_activity), t('@citations citations', array('@citations' => $citations)));
  }

  $headers = array(t('Name'), t('Department'), t('Email'), t('Phone Number'), t('Research Interests'), t('Citations'));

  $form['search'] = array(
    '#type' => 'textfield',
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Search'),
  );

  $form['author'] = array(
    '#value' => theme('table', $headers, $users),
  );

  return $form;
}

function islandora_scholar_browse_by_author_form_submit($form, &$form_state) {

  drupal_goto('browse_authors2/' . $form_state['values']['search']);

}