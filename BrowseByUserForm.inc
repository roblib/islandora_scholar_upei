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
  $researchers = array();
  $result = file_get_contents('http://localhost:8080/solr/select/?q=rels.hasModel:researcherCModel&fl=mads.family,mads.given&rows=10000');
  $xml = new SimpleXMLElement($result);
  $array = $xml->xpath('//doc');
  foreach ($array as $arr) {
    $family = $arr->xpath('./arr[@name="mads.family"]/str');
    $given = $arr->xpath('./arr[@name="mads.given"]/str');
    $researchers[(string) $given[0] . ' ' . (string) $family[0]] = (string) $family[0] . ', ' . (string) $given[0];
    }

  asort($researchers);
  
  $form['author'] = array(
      '#title' => t('Browse by author'),
      '#type' => 'select',
      '#options' => $researchers,
    );
  
    $form['author_submit'] = array(
      '#type' => 'submit',
      '#value' => t('Browse'),
      '#button' => 'browse',
    );
  
  return $form;
}

/**
 * Submit hook for the browse by user form.
 * 
 * @param array $form
 * @param array $form_state 
 */
function scholar_browse_by_user_form_submit(array $form, array &$form_state) {
  $user = $form_state['values']['author'];
  drupal_set_message('User: ' . $user);
  drupal_goto('islandora/solr/search/mods.author:' . $user);
}