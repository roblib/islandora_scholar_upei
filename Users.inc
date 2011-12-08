<?php

/**
 * @file
 * Functions to help in the creationg of users or researcher authority records
 */
function scholar_ldap_users_search_form() {

  $form['search'] = array(
    '#type' => 'textfield',
    '#title' => t('Search'),
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Submit'),
  );

  return $form;
}

function scholar_ldap_users_search_form_submit($form, &$form_state) {
  $query = htmlentities($form_state['values']['search']);
  drupal_goto('add_users/' . $query);
}

function scholar_get_ldap_users($query) {
  module_load_include('inc', 'scholar', 'UsersClass');
  $ldap_host = variable_get('scholar_ldap_host', '');
  $ldap_port = variable_get('scholar_ldap_port', '389');
  $ds = ldap_connect($ldap_host, $ldap_port) or die("Could not connect to ldap host!");

  $basedn = variable_get('scholar_ldap_basedn', '');

  $filter = "(&(objectClass=person)(uid=*)(fullName=$query))";
  $attr = array("dn", "cn", "mail");

  $sr = ldap_list($ds, $basedn, $filter);

  $list = ldap_get_entries($ds, $sr);

  $count = $list['count'];

  if ($count == 0) {
    return NULL;
  }

  for ($i = 0; $i < $count; $i++) {

    $users[$i]['uid'] = $list[$i]['uid'][0];
    $users[$i]['cn'] = $list[$i]['cn'][0];
    $users[$i]['fullname'] = $list[$i]['fullname'][0];
    $users[$i]['mail'] = $list[$i]['mail'][0];
    $usernames[$i] = $list[$i]['cn'][0];
  }

  $headers = array(
    array('data' => t('Full name')),
    array('data' => t('UID')),
    array('data' => t('cn')),
    array('data' => t('Mail')),
  );
// Todo: work out a way of checking and displaying if the users have already been added
// Method using the ITQL query takes a long time, maybe use solr?
  $table = array(
//    '#header' => array(theme('table_select_header_cell'), t('Full name'), t('Email'), t('Added'), ''),
    '#header' => array(theme('table_select_header_cell'), t('Full name'), t('Email'), ''),
    '#theme' => THEME_BIBLIOGRAPHY_FORM_TABLE,
    '#tree' => TRUE,
    'rows' => array(),
    'selections' => array(
      '#type' => 'checkboxes',
      '#options' => array_fill_keys($usernames, ''),
    ),
  );
  $rows = &$table['rows'];
  foreach ($users as $user) {
//    $entry = new UsersClass($user['cn']);
//      $check = $entry->check_user();
//    $check = FALSE;
//      if ($check == TRUE) {
//        $image = '<img src="/misc/watchdog-ok.png"/>';
//      } 
//      else {
//        $image = '';
//      }
    $rows[] = array(
      '#pid' => $user['cn'],
      'fullname' => array('#value' => check_plain($user['fullname'])),
      'mail' => array('#value' => check_plain($user['mail'])),
//      'added' => array('#value' => $image),
    );
  }
  return $table;
}

function scholar_get_ldap_users_form($form, $query) {
  $form = array();

  $table = scholar_get_ldap_users($query);

  if ($table != NULL) {

    $form['table'] = array(
      'table' => $table,
    );

    $form['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Submit'),
    );
  }
  else {
    drupal_set_message(t('No results were found!'), 'warning');
  }
  return $form;
}

function scholar_get_ldap_users_form_submit($form, &$form_state) {
  module_load_include('inc', 'islandora_content_model_forms', 'FOXML');
  module_load_include('inc', 'fedora_repository', 'CollectionPolicy');
  module_load_include('inc', 'scholar', 'UsersClass');
  $users = array_filter($form_state['values']['table']['selections']);
//  $max = count($users);
  $max = 1;
  if ($collection_policy !== FALSE) {
    
    foreach ($users as $user) {
//  $result = in_array($user, $context['results']);
//  if (!$result === TRUE) {
    $details = new UsersClass($user);
    $check = $details->check_user();
    if ($check !== TRUE) {
    $collection_pid = 'ir:authorityCollection';
    $content_model_pid = 'ir:researcherCModel';
    $transform = drupal_get_path('module', 'scholar') . '/xsl/mads_to_dc.xsl';
    $collection_policy = CollectionPolicy::loadFromCollection($collection_pid);
    
    $xml = $details->ldap_to_mads();
    $dir = $_SERVER['DOCUMENT_ROOT'] . base_path() . file_directory_path();
    $filename = $dir . '/mads.xml';
    $xmlstr = $xml->save($filename);

    $label = $details->cn;
    $pid = $collection_policy->getNextPid('ISLANDORACM');
    $dsid = 'MADS';
    $relationship = $collection_policy->getRelationship();
    $ingest_file_locations = array();
    $document = $xml;
    
      $foxml = new FOXML($label, $pid, $dsid, $content_model_pid, $collection_pid, $relationship, $ingest_file_locations, $document, $transform);
      $foxml->ingest();
    }
  }
    
//    foreach ($users as $user) {
//      $operations[] = array(
//        'scholar_batch_user_add',
//        array(
//          $users,
//          $max,
//        ),
//      );
//      $batch = array(
//        'operations' => $operations,
//        'file' => drupal_get_path('module', 'scholar') . '/Users.inc',
//        'finished' => 'scholar_batch_user_add_finished',
//        'title' => t('Importing users'),
//        'error_message' => t('An error occurred'),
//      );
//      batch_set($batch);
    }
//  }
}

function scholar_batch_user_add($users, $max, &$context) {
  module_load_include('inc', 'islandora_content_model_forms', 'FOXML');
  module_load_include('inc', 'fedora_repository', 'CollectionPolicy');
  module_load_include('inc', 'scholar', 'UsersClass');
  $set = isset($context['sandbox']['progress']);
  if ($set == FALSE) {
    $context['sandbox']['progress'] = 0;
    $context['sandbox']['max'] = $max;
    $context['sandbox']['users'] = array();
  }
  foreach ($users as $user) {
  $result = in_array($user, $context['results']);
  if (!$result === TRUE) {
    $details = new UsersClass($user);
    $check = $details->check_user();
    if ($check !== TRUE) {
    $collection_pid = 'ir:authorityCollection';
    $content_model_pid = 'ir:researcherCModel';
    $transform = drupal_get_path('module', 'scholar') . '/xsl/mads_to_dc.xsl';
    $collection_policy = CollectionPolicy::loadFromCollection($collection_pid);
    
    $xml = $details->ldap_to_mads();
    $dir = $_SERVER['DOCUMENT_ROOT'] . base_path() . file_directory_path();
    $filename = $dir . '/mads.xml';
    $xmlstr = $xml->save($filename);

    $label = $details->cn;
    $pid = $collection_policy->getNextPid('ISLANDORACM');
    $dsid = 'MADS';
    $relationship = $collection_policy->getRelationship();
    $ingest_file_locations = array();
    $document = $xml;
    
      $foxml = new FOXML($label, $pid, $dsid, $content_model_pid, $collection_pid, $relationship, $ingest_file_locations, $document, $transform);
      $foxml->ingest();
    }
  }
  $context['sandbox']['progress']++;
  $context['sandbox']['users'][] = $user;
  $context['results'][] = $user;
  if ($context['sandbox']['progress'] == $context['sandbox']['max']) {
// Done!
    $context['finished'] = 1;
  }
  else {
    $context['finished'] = ($context['sandbox']['progress'] / $context['sandbox']['max']);
  }
  }
}

function scholar_batch_user_add_finished($success, $results, $operations) {
  if ($success) {
//    drupal_set_message(t('@count users imported', array("@count" => count($results))));
  }
  else {
    // A fatal error occurred during batch processing.
    $error_operation = reset($operations);
    $operation = array_shift($error_operation);
    $arguments = array_shift($error_operation);
    $arguments_as_string = implode(', ', $arguments);
    watchdog('importusers', "Error when calling operation '%s'('%s')", array($operation, $arguments_as_string));
    drupal_set_message(t('An error occurred and has been recorded in the system log.'), 'error');
  }
}