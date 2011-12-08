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
function scholar_browse_by_user_form(array &$form_state) {
  $users = array();
//  $result = db_query("SELECT uid, name FROM {users} WHERE uid > 1");
//  while ($user = db_fetch_object($result)) {
//    $users[$user->uid] = $user->name;
//  }
  
  $result = file_get_contents('http://localhost:8080/solr/select/?q=rels.hasModel:researcherCModel&fl=mads.given,mads.family');
  
  $xml = new SimpleXMLElement($result);
  
  $users = array_unique($xml->xpath('//doc/arr/str'));
 
  foreach ($users as $user) {
    $output .= "<li><a href=islandora/solr/search/mads.family:$user>" . $user . '</a></li>';
  }
  
  $form['user'] = array(
      '#type' => 'select',
      '#options' => $users,
    );
  
  $form['submit'] = array(
      '#type' => 'submit',
      '#value' => 'Browse',
    );
  
    var_dump($form);
  
  return $form;
}

/**
 * Submit hook for the browse by user form.
 * 
 * @param array $form
 * @param array $form_state 
 */
function scholar_browse_by_user_form_submit(array $form, array &$form_state) {
  drupal_goto('/islandora/solr/search/test');
}