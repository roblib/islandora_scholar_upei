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
  module_load_include('inc', 'islandora_bibliography', 'Bibliography');
  $pid = isset($pid) ? $pid : $form_state['post']['pid'];
  $stored = isset($form_state['post']['stored']) ? $form_state['post']['stored'] : Bibliography::Exists($pid);
  return array(
    '#action' => '/bibliography/citation',
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
      '#value' => $stored ? t('Remove from Bibliography') : t('Add to Bibliography'),
      '#action' => $stored ? 'remove' : 'add',
      '#pid' => $pid,
      '#redirect' => 'fedora/repository/' . $pid,
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
  ($action == 'add') ?
          Bibliography::AddCitation($pid) :
          Bibliography::RemoveCitation($pid);
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
  $path = drupal_get_path('module', 'scholar');
  $pid = isset($pid) ? $pid : $form_state['post']['pid'];
  $stored = isset($form_state['post']['stored']) ? $form_state['post']['stored'] : Bibliography::Exists($pid);
  var_dump($stored);
  $redirect = ($_SESSION['search_url']);
    $redirect = str_replace(array('%253A', '%2529', '%2528', '%252F', '%20', '%2522', '%3A', '%2520'), array(':', '', '', '', ' ', '"', ':', ' '), $redirect);
  return array(
    '#action' => '/bibliography/citation/search',
    'pid' => array(
      '#type' => 'hidden',
      '#value' => $pid
    ),
    'stored' => array(
      '#type' => 'hidden',
      '#value' => $stored
    ),
    'action' => array(
      '#type' => 'image_button',
      '#src' => $stored ? $path . '/images/bookmarks_list_remove2.png' : $path . '/images/bookmarks_list_add.png',
      '#value' => $stored ? t('Remove from Bibliography') : t('Add to Bibliography'),
      '#action' => $stored ? 'remove' : 'add',
      '#pid' => $pid,
      '#redirect' => $redirect,
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
function islandora_bibliography_citation_search_form_submit(array $form, array &$form_state) {
  module_load_include('inc', 'islandora_bibliography', 'Bibliography');
  $action = $form_state['clicked_button']['#action'];
  $pid = $form_state['clicked_button']['#pid'];
  ($action == 'add') ?
          Bibliography::AddCitation($pid) :
          Bibliography::RemoveCitation($pid);
  $redirect = $form_state['clicked_button']['#redirect'];
  drupal_goto($redirect);
}