<?php

/**
 * @file
 *
 * Home page for the IR.
 */

/**
 * Menu callback for the Scholar modules landing page.
 * 
 * Renders the Home Page.
 * 
 * @return string
 *   HTML for Institutional Repository Home Page.
 */
function scholar_home() {
  module_load_include('inc', 'scholar', 'SearchForm');
  module_load_include('inc', 'scholar', 'BrowseByUserForm');
  return theme(THEME_SCHOLAR_HOME);
}

/**
 * Implements the Theme Hook function for the ir_home theme.
 *
 * @param $variables
 */
function template_preprocess_scholar_home(array &$variables) {
  $path = drupal_get_path('module', 'scholar');
  drupal_add_css("$path/css/Home.css");
  $variables['browse_by_user'] = db_result(db_query("SELECT COUNT(uid) FROM {users} WHERE uid > 1")) > 0;
  $variables['browse_by_department'] = TRUE;
}