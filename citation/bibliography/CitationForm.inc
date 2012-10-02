<?php

/**
 * @file
 *
 * Functions for including the Add/Remove Citation form.
 */

/**
 * Renders the form for adding/removing an item from the bibliography.
 *
 * @param array $form_state
 *   The drupal form state.
 * @param string $pid
 *   The fedora object's pid.
 *
 * @return array
 *   The drupal form.
 */
function islandora_bibliography_citation_form(array &$form_state, $pid = NULL) {
  global $base_url;
  module_load_include('inc', 'islandora_bibliography', 'Bibliography');
  $pid = isset($pid) ? $pid : $form_state['post']['pid'];
  $stored = isset($form_state['post']['stored']) ? $form_state['post']['stored'] : Bibliography::Exists($pid);
  return array(
    '#action' => $base_url . '/bibliography/citation',
    'pid' => array(
      '#type' => 'hidden',
      '#value' => $pid
    ),
    'stored' => array(
      '#type' => 'hidden',
      '#value' => $stored
    ),
    'action' => array(
      '#type' => 'submit',
      '#value' => $stored ? t('Remove from My List') : t('Add to My List'),
      '#action' => $stored ? 'remove' : 'add',
      '#pid' => $pid,
      '#redirect' => $base_url . '/fedora/repository/' . $pid,
    )
  );
}

/**
 * Adds/Removes a Citation from the bibliography.
 *
 * @param array $form
 *   The drupal form.
 * @param array $form_state
 *   The drupal form state.
 */
function islandora_bibliography_citation_form_submit(array $form, array &$form_state) {
  module_load_include('inc', 'islandora_bibliography', 'Bibliography');
  $action = $form_state['clicked_button']['#action'];
  $pid = $form_state['clicked_button']['#pid'];
  if ($action == 'add') {
    $return = Bibliography::AddCitation($pid);
  }
  else {
    $return = Bibliography::RemoveCitation($pid);
  }
  $redirect = $form_state['clicked_button']['#redirect'];
  drupal_goto($redirect);
}

/**
 * Renders the form for adding/removing an item from the bibliography.
 *
 * @param array $form_state
 *   The drupal form state.
 * @param string $pid
 *   The fedora object's pid.
 *
 * @return array
 *   The drupal form.
 */
function islandora_bibliography_citation_search_form(array &$form_state, $pid = NULL) {
  module_load_include('inc', 'islandora_bibliography', 'Bibliography');
  global $base_url;
  //drupal_add_js(drupal_get_path('module', 'scholar') . '/js/ahah_helper.js', 'module', 'footer');
  //drupal_add_js(drupal_get_path('module', 'scholar') . '/js/searchresults.js');
  //drupal_add_js(drupal_get_path('module', 'scholar') . '/js/Ahah.js');
  $pid = isset($pid) ? $pid : $form_state['post']['pid'];
  $css_id = str_replace(':', '-', $pid);
  $stored = isset($form_state['values']['stored']) ? $form_state['values']['stored'] : Bibliography::Exists($pid);
  $form =  array(
    '#action' => $base_url . '/bibliography/',
    'pid' => array(
      '#type' => 'hidden',
      '#value' => $pid
    ),
    'stored' => array(
      '#type' => 'hidden',
      '#value' => $stored
    ),
    'procedure' => array(
      '#tree' => TRUE,
      '#prefix' => '<div id="my-list-add-from-search-' . $css_id . '">',
      '#suffix' => '</div>',
      '#button_type' => $stored ? t('my-list-exists') : t('my-list-add'),
      '#type' => 'button',
      //'#src' => $stored ? $path . '/images/bookmarks_list_remove2.png' : $path . '/images/bookmarks_list_add.png',
      '#value' => $stored ? t('Exists in My List') : t('Add to My List'),
      //'#disabled' => $stored ,
      '#pid' => $pid,
      //'#redirect' => $redirect,
      
    ),
     
  );
  if(!$stored){
   $form['procedure']['#ahah'] = array(
        'path' => 'bibliography/citation/search/add' ,
        'wrapper' => 'my-list-add-from-search-' . $css_id,
        'method' => 'replace',
        'effect' => 'fade'
      );
  }
  return $form;
}

/**
 * Adds/Removes a Citation from the bibliography.
 *
 * @param array $form
 *   The drupal form.
 * @param array $form_state
 *   The drupal form state.
 */
function islandora_bibliography_citation_search_form_submit(array $form, array &$form_state) {
  module_load_include('inc', 'islandora_bibliography', 'Bibliography');
  //$action = $form_state['clicked_button']['#action'];
  //$pid = $form_state['clicked_button']['#post']['pid'];
  //($action == 'add') ?
  //        Bibliography::AddCitation($pid) :
  //        Bibliography::RemoveCitation($pid);
  //$redirect = $form_state['clicked_button']['#redirect'];
  drupal_goto('bibliography');
}