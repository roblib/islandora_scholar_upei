<?php

/**
 * @file
 * Functions to help in the creationg of users or researcher authority records
 */

/**
 * Form to search for specific users in the LDAP directory
 * @return string 
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

/**
 * Takes the value from the search form and redirects to
 * the add_users form with the value as an argument
 * @param type $form
 * @param type $form_state 
 */
function scholar_ldap_users_search_form_submit($form, &$form_state) {
  $query = htmlentities($form_state['values']['search']);
  drupal_goto('add_users/' . $query);
}

/**
 * Function that queries the LDAP server based on the value passed and
 * displays the results in a table. The results can be selected and objects 
 * with MADS datastreams will be created in fedora.
 * 
 * @param type $query
 * Value entered in the LDAP search form. Can have wildcards (eg r* will
 * bring back all the ldap entries beginning with r)
 * @return array 
 */
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
    $rows[] = array(
      '#pid' => $user['cn'],
      'fullname' => array('#value' => check_plain($user['fullname'])),
      'mail' => array('#value' => check_plain($user['mail'])),
    );
  }
  return $table;
}

/**
 * Form to display the LDAP users table
 * @param type $form
 * @param type $query
 * @return string 
 */
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

/**
 * This function builds the MADS file from data in the LDAP entry and
 * adds it to the researcher object.
 * @todo Get a batch process working with this. At the moment the process
 * is fairly quick but could take a while depending on how many users are 
 * being added
 * @param type $form
 * @param type $form_state 
 */
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
        $collection_pid = 'ir:people';
        $content_model_pid = 'ir:researcherCModel';
        $transform = drupal_get_path('module', 'scholar') . '/xsl/mads_to_dc.xsl';
        $collection_policy = CollectionPolicy::loadFromCollection($collection_pid);

        $xml = $details->ldap_to_mads();
        $dir = $_SERVER['DOCUMENT_ROOT'] . base_path() . file_directory_path();
        $filename = $dir . '/mads.xml';
        $xmlstr = $xml->save($filename);

        $label = $details->cn;
        $pid = 'ir:' . strtolower($label);
        $dsid = 'MADS';
        $relationship = $collection_policy->getRelationship();
        $ingest_file_locations = array();
        $document = $xml;

        $foxml = new FOXML($label, $pid, $dsid, $content_model_pid, $collection_pid, $relationship, $ingest_file_locations, $document, $transform);
        $foxml->ingest();
        // TODO: add thumbnail external reference
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

/**
 * Function to process the LDAP users as a batch. Took much longer than
 * doing it without the batch process so I took it out. Obviously needs 
 * some tuning
 * @param type $users
 * @param type $max
 * @param type $context 
 */
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
        $collection_pid = 'ir:people';
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

function scholar_author_bibliography_form(array &$form_state, array $pids, $username) {
  module_load_include('inc', 'islandora_bibliography', 'Bibliography');
  module_load_include('inc', 'islandora_bibliography', 'BibliographyForm');
  module_load_include('inc', 'citation_exporter', 'Batch');
  module_load_include('module', 'citation_exporter', 'citation_exporter');
  module_load_include('inc', 'citation_exporter', 'BatchExportRIS');
  module_load_include('inc', 'citation_exporter', 'BatchExportRTF');
  module_load_include('inc', 'citation_exporter', 'BatchExportPDF');

  citation_exporter_include_download_redirect();
//  $pids = Bibliography::GetCitations();
  if (empty($pids)) {
    islandora_bibilgraphy_form_empty_page();
  }
  else {
    module_load_include('inc', 'csl', 'CSL');
    drupal_add_css(drupal_get_path('module', 'islandora_bibliography') . '/css/bibliography.css');
    $styles = CSL::GetNames();
    return array(
      'step_1' => array(
        '#value' => t('Step 1: Select your citations'),
        '#prefix' => '<h3>',
        '#suffix' => '</h3>'
      ),
      'table_div_open' => array(
        '#value' => '<div class="bibliography-table">',
      ),
      'table' => islandora_bibilgraphy_form_table($pids),
      'table_div_close' => array(
        '#value' => '</div>',
      ),
      'step_3' => array(
        '#value' => t('Step 2: Preview or export the formatted citations'),
        '#prefix' => '<h3>',
        '#suffix' => '</h3>'
      ),
      'format' => array(
        '#title' => t('Format'),
        '#type' => 'select',
        '#length' => 50,
        '#options' => array(
          'RIS' => 'RIS',
          'RTF' => 'RTF',
          'PDF' => 'PDF',
        )
      ),
      'style' => array(
        '#title' => t('Style'),
        '#type' => 'select',
        '#options' => $styles
      ),
      'preview' => array(
        '#type' => 'submit',
        '#prefix' => '</br>',
        '#value' => t('Preview Selected'),
        '#submit' => array('scholar_author_bibliography_form_preview')
      ),
      'export' => array(
        '#type' => 'submit',
        '#action' => 'export',
        '#export' => 'selected',
        '#value' => t('Export Selected'),
        '#submit' => array('islandora_bibliography_form_export')
      ),
      'username' => array(
        '#type' => 'hidden',
        '#value' => $username,
      ),
    );
  }
}

function scholar_author_bibliography_form_preview(array $form, array &$form_state) {
  module_load_include('inc', 'citeproc', 'CiteProcJSBibliography');
  drupal_add_css(drupal_get_path('module', 'islandora_bibliography') . '/css/export.css');
  $selections = $form_state['values']['table']['selections'];
  $username = strtolower($form_state['values']['username']);
  $style = citeproc_style($form_state['values']['style']);
  $bibliography = new CiteProcJSBibliography($style);
  $selections = array_filter($selections);
  $selections = array_keys($selections);
  foreach ($selections as $pid) {
    $mods = islandora_bibilgraphy_form_get_mods($pid);
    if ($mods) {
      $citation = citeproc_citation_from_mods($mods);
      $bibliography->addCitation($citation);
    }
  }
  if (isset($selections[0])) {
    $output = $bibliography->render();
  }
  else {
    drupal_set_message(t('No citations were selected!'), 'warning');
  }
  $output .= l(t('Return to author.'), 'fedora/repository/ir:' . $username);
  print theme('page', $output, FALSE);
  exit();
}

function scholar_author_metrics_form($username) {
  global $base_url;
  global $base_path;
  global $user;
  module_load_include('inc', 'fedora_repository', 'api/fedora_item');
  module_load_include('inc', 'scholar_tracking', 'scholar_tracking');
  module_load_include('inc', 'scholar', 'Users');

  $page = isset($_GET['page']) ? $_GET['page'] : '0';
  $items_per_page = 5;
  $offset = $page * $items_per_page;
  
  $username_query = file_get_contents('http://localhost:8080/solr/select/?q=mods.username:' . (str_replace(';', ' ', (strtolower($username)))) . '&fl=PID&rows=10000');
  $xml = new SimpleXMLElement($username_query);
  $array = $xml->xpath('//doc');
  $array = array_filter($array);
  $rows = array();
  if (!empty($array)) {
    if (module_exists('scholar_tracking')) {
      $header = array(t('Title'), t('Times viewed'), t('Last viewed'));
    }
    else {
      $header = array(t('Title'));
    }
    $no_of_rows = count($array);
    for ($i = $offset; $i < $offset + $items_per_page; $i++) {
      $pid = (string) $array[$i]->str;
      $pids[] = $pid;
      $object = new CollectionClass();
      $pid_xml_str = $object->getStream($pid, "MODS");
      $pid_xml = new SimpleXMLElement($pid_xml_str);
      $title = $pid_xml->titleInfo->title;
      if (module_exists('scholar_tracking')) {
        $track = new islandora_scholar_tracking($pid);
        $read = $track->read_page_count();
        $time_last_viewed = $read[($read['count'] - 1)];
        if ($time_last_viewed == NULL) {
          $time_last_viewed = 'Never';
        }
        $rows[] = array(l($title, 'fedora/repository/' . $pid, array('html' => TRUE)));
      }
      else {
        $rows[] = array(l($title, 'fedora/repository/' . $pid, array('html' => TRUE)), t('@read', array('@read' => $read['count'])), $time_last_viewed);
      }
    }
  }
  else {
    $rows = NULL;
  }
  
  $no_of_items = count($rows);
  $total = $no_of_items / $items_per_page;
  $element = 0;
  $GLOBALS['pager_page_array'][$element] = max(0, min((int) $pager_page_array[$element], ((int) $pager_total[$element]) - 1));
  $GLOBALS['pager_total'][$element] = ceil($items_per_page / $no_of_items);
  
  pager_query("SELECT %d", $items_per_page, 0, NULL, $total);
  
//      $cache_time = variable_get('scholar_researcher_citations_cache_time', '2');
//      cache_set('islandora_scholar_citation_cache', serialize($rows), 'cache', time() + ($cache_time * 86400));
//    }

  if ($rows == NULL) {
    $content = t('No citations found!');
  }
  else {
    $content = theme('table', $header, $rows);
  }
  
  $output = $content . theme('pager');

  return $output;
}