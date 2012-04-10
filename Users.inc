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
 * Function to query LDAP
 * @param type $query Can be a string or array. Works best with * at the start and
 * end of the query (eg *username*)
 * @return type 
 */
function islandora_scholar_ldap_search($query) {
  $ldap_host = variable_get('scholar_ldap_host', '');
  $ldap_port = variable_get('scholar_ldap_port', '389');
  $ds = ldap_connect($ldap_host, $ldap_port) or die("Could not connect to ldap host!");

  $basedn = variable_get('scholar_ldap_basedn', '');

  if (is_array($query)) {
    $filter = "(&(objectClass=person)(uid=*)(|";
    foreach ($query as $q) {
      $filter .= "(fullName=* $q *)";
    }
    $filter .= "))";
  }
  else {
    $filter = "(&(objectClass=person)(uid=*)(fullName=$query))";
  }

  $attr = array("dn", "cn", "mail");

  $sr = ldap_search($ds, $basedn, $filter, array('uid', 'cn', 'fullname', 'mail', 'groupmembership'));

  $list = ldap_get_entries($ds, $sr);

  $count = $list['count'];
  if ($count == 0) {
    return NULL;
  }
  else {
    return $list;
  }
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

  $list = islandora_scholar_ldap_search($query);

  if ($list == NULL) {
    return $list;
  }

  $count = $list['count'];

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

  $table = array(
    '#header' => array(theme('table_select_header_cell', array(1)), t('Full name'), t('Email'), t('Imported'), ''),
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
    $details = new UsersClass($user['cn']);
    $details->check_user() ? $user_image = '<img src="' . url('misc/watchdog-ok.png') . '"/>' : $user_image = '<img src="' . url('misc/watchdog-error.png') . '"/>';
    $rows[] = array(
      '#pid' => $user['cn'],
      'fullname' => array('#value' => check_plain($user['fullname'])),
      'mail' => array('#value' => check_plain($user['mail'])),
      'image' => array('#value' => $user_image),
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
//    drupal_goto('search_users');
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

        if ($details->fullname != NULL) {
          $label = $details->fullname;
        }
        else {
          $label = $details->cn;
        }

        $pid = 'ir:' . strtolower($details->cn);
        $dsid = 'MADS';
        $relationship = $collection_policy->getRelationship();
        $ingest_file_locations = array();
        $document = $xml;

        $foxml = new FOXML($label, $pid, $dsid, $content_model_pid, $collection_pid, $relationship, $ingest_file_locations, $document, $transform);
        $foxml->ingest();

        $item = new Fedora_Item($pid);
        $groups = $details->groupmembership;

        $depts = array();

        if ($groups != NULL) {
          foreach ($groups as $group) {
            $dept = strtok($group, ',');
            $dept = ltrim($dept, 'cn=');
            $dept = islandora_scholar_convert_dept($dept);
            $dept = str_replace(' ', '_', $dept);
            if ($dept) {
              $depts[] = $dept;
            }
          }
          $depts = array_unique($depts);
          foreach ($depts as $dept) {
            $item->add_relationship('hasDepartment', $dept);
          }
        }
        $thumbnail = drupal_get_path('module', 'scholar') . '/images/gnome_stock_person.png';
        $item->add_datastream_from_file($thumbnail, 'TN', 'Thumbnail');
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

function scholar_author_bibliography_form(array &$form_state, $names, $username, $type) {
  module_load_include('inc', 'islandora_bibliography', 'Bibliography');
  module_load_include('inc', 'islandora_bibliography', 'BibliographyForm');
  module_load_include('inc', 'citation_exporter', 'Batch');
  module_load_include('module', 'citation_exporter', 'citation_exporter');
  module_load_include('inc', 'citation_exporter', 'BatchExportRIS');
  module_load_include('inc', 'citation_exporter', 'BatchExportRTF');
  module_load_include('inc', 'citation_exporter', 'BatchExportPDF');

  citation_exporter_include_download_redirect();
  $table = islandora_bibilgraphy_solr_form_table($names, $type);
//  $pids = Bibliography::GetCitations();
  if (empty($names) || $table == NULL) {
//    islandora_bibilgraphy_form_empty_page();
    return array(
      'no_citations' => array(
        '#value' => t('No citations found!'),
      ),
    );
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
      'table' => $table,
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
          'RTF' => 'RTF',
          'PDF' => 'PDF',
          'RIS' => 'RIS',
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

function islandora_bibilgraphy_solr_form_table($names, $type) {

  switch ($type) {
    case 'author-citations':
      $url = 'http://localhost:8080/solr/select?q=rels.hasModel:citation%20AND%20mods.username:"' . (str_replace(array(';', ' '), array(' ', '%20'), ($names))) . '"&fl=PID,mods.title,mods.author,author&rows=10000';
      $query = file_get_contents($url);
      break;
    case 'department-citations':
      $url = 'http://localhost:8080/solr/select?q=rels.hasModel:citation%20AND%20mods.department:"' . (str_replace(array(';', ' '), array(' ', '%20'), ($names))) . '"&fl=PID,mods.title,mods.author,author&rows=10000';
      $query = file_get_contents($url);
      break;
    case 'author-theses':
      $url = 'http://localhost:8080/solr/select?q=rels.hasModel:thesis%20AND%20mods.username:"' . (str_replace(array(';', ' '), array(' ', '%20'), ($names))) . '"&fl=PID,mods.title,mods.author,author&rows=10000';
      $query = file_get_contents($url);
      break;
    case 'department-theses':
      $url = 'http://localhost:8080/solr/select?q=rels.hasModel:thesis%20AND%20mods.department:"' . (str_replace(array(';', ' '), array(' ', '%20'), ($names))) . '"&fl=PID,mods.title,mods.author,author&rows=10000';
      $query = file_get_contents($url);
      break;
  }

  if ($query != NULL) {
    $xml = new SimpleXMLElement($query);
    $array = $xml->xpath('//doc');
    $array = array_filter($array);
    $pids = array();
    $titles = array();
    $authors = array();
    foreach ($array as $arr) {
      $author_array = array();
      $pid = (string) $arr->str;
      $pids[] = $pid;
      if ($arr->arr[0]['name'] == 'author' || $arr->arr[0]['name'] == 'mods.author') {
        foreach ($arr->arr[0] as $author) {
          $author_array[] = str_replace(',', ', ', $author);
        }
        $authors[$pid] = implode('; ', $author_array);
      }
      else {
        $authors[$pid] = t('Missing');
      }
      if ($arr->arr[1]['name'] == 'mods.title') {

        $titles[$pid] = ($arr->arr[1]->str);
      }
      else {
        $titles[$pid] = t('Missing');
      }
    }
    $table = array(
      '#header' => array(theme('table_select_header_cell', array(1)), t('Title'), t('Author(s)'), ''),
      '#theme' => THEME_BIBLIOGRAPHY_FORM_TABLE,
      '#tree' => TRUE,
      'rows' => array(),
      'selections' => array(
        '#type' => 'checkboxes',
        '#options' => array_fill_keys($pids, ''),
        '#value' => array_fill_keys($pids, '1'),
      ),
    );
    $rows = &$table['rows'];

    if (empty($pids)) {
      return NULL;
    }

    foreach ($pids as $pid) {
//    list($title, $authors) = islandora_bibilgraphy_solr_form_get_title_authors($pid);
      $rows[] = array(
        '#pid' => $pid,
        'title' => array('#value' => l($titles[$pid], 'fedora/repository/' . $pid, array('html' => TRUE))),
        'author' => array('#value' => check_plain($authors[$pid])),
      );
    }
    return $table;
  }
}

/**
 * Get the title and author of a given citation.
 * 
 * @param string $pid
 *   A citations within the bibliography.
 * 
 * @return array
 *   Where the first key is the title and the second is the authors.
 */
function islandora_bibilgraphy_solr_form_get_title_authors($pid) {
  $title = $authors = 'Missing';

  $solr_query = 'http://localhost:8080/solr/select/?q=PID:' . (str_replace(':', '?', ($pid))) . '&fl=dc.title,author&rows=10000';
  $solr_result = file_get_contents($solr_query);
  $xml = new SimpleXMLElement($solr_result);
  $array = $xml->xpath('//doc');
  $array = array_filter($array);
  $names = array();
  foreach ($array as $arr) {
    if ($arr->arr[0]['name'] == 'author') {
      $names_array = $arr->arr[0]->str;
      foreach ($names_array as $name) {
        $names[] = str_replace(',', ', ', (string) $name);
      }
      $authors = implode('; ', $names);
    }
    if ($arr->arr[1]['name'] == 'dc.title') {
      $title = $arr->arr[1]->str;
    }
  }
  return array($title, $authors);
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

function scholar_object_metrics_form($identifier, $type) {
  global $base_url;
  global $base_path;
  global $user;
  module_load_include('inc', 'fedora_repository', 'api/fedora_item');
  module_load_include('inc', 'scholar_tracking', 'scholar_tracking');
  module_load_include('inc', 'scholar', 'Users');
  drupal_add_css(drupal_get_path('module', 'scholar') . '/scholar.css');

  switch ($type) {
    case 'author':
      $query_type = 'mods.username';
      break;
    case 'department':
      $query_type = 'mods.department';
      break;
    default:
      $query_type = 'mods.username';
      break;
  }
  $solr_query = 'http://localhost:8080/solr/select/?q=' . $query_type . ':"' . (str_replace(';', ' ', ($identifier))) . '"&fl=PID,mods.title&rows=10000';
  $username_query = @file_get_contents($solr_query);
  if ($username_query != NULL) {
    $xml = new SimpleXMLElement($username_query);
    $array = $xml->xpath('//doc');
    $array = array_filter($array);
//  $title = $xml->xpath("//doc/arr['mods.title']");
//  var_dump($title[0]->str);
    $rows = array();
    if (!empty($array)) {
      if (module_exists('scholar_tracking')) {
        $header = array(t('Title'), t('Times viewed'), t('Last viewed'));
      }
      else {
        $header = array(t('Title'));
      }
      $no_of_rows = count($array);
      $range = $offset + $items_per_page;
      if ($range > $no_of_rows) {
        $range = $offset + ($no_of_rows - $offset);
      }
      foreach ($array as $arr) {
        $pid = (string) $arr->str;
        $pids[] = $pid;
        $title = $arr->arr->str;
//      $object = new CollectionClass();
//      $pid_xml_str = $object->getStream($pid, "MODS");
//      $pid_xml = new SimpleXMLElement($pid_xml_str);
//      $title = $pid_xml->titleInfo->title;
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
  }
  else {
    $rows = NULL;
  }

//      $cache_time = variable_get('scholar_researcher_citations_cache_time', '2');
//      cache_set('islandora_scholar_citation_cache', serialize($rows), 'cache', time() + ($cache_time * 86400));
//    }

  if ($rows == NULL) {
    $content = t('No citations found!');
  }
  else {
    $content = theme('table', $header, $rows);
  }

  return $content;
}

function islandora_scholar_researcher_compare_form() {

  $form = array();

  $existing_researchers_query = 'http://localhost:8080/solr/select?q=rels.hasModel:researcher&fl=mads.username&rows=10000';
  $existing_researchers_result = @file_get_contents($existing_researchers_query);

  if ($existing_researchers_result != NULL) {
    $xml = new SimpleXMLElement($existing_researchers_result);
    $array = $xml->xpath('//doc');
    $existing_researchers = array();
    foreach ($array as $arr) {
      $name = $arr->xpath('./arr[@name="mads.username"]/str');
      foreach ($name as $n) {
        if ($n) {
          $existing_researchers[] = strtolower($n[0]);
        }
      }
    }
  }

//  var_dump(array_unique($existing_researchers));

  $citation_researchers_query = 'http://localhost:8080/solr/select?q=rels.hasModel:citation&fl=mods.username&rows=10000';
  $citation_researchers_result = @file_get_contents($citation_researchers_query);

  if ($citation_researchers_result != NULL) {
    $xml = new SimpleXMLElement($citation_researchers_result);
    $array = $xml->xpath('//doc');
    $citation_researchers = array();
    foreach ($array as $arr) {
      $name = $arr->xpath('./arr[@name="mods.username"]/str');
      foreach ($name as $n) {
        if ($n) {
          $citation_researchers[] = strtolower($n[0]);
        }
      }
    }
  }

//  var_dump(array_unique($citation_researchers));

  $diff = array_diff(array_unique($citation_researchers), array_unique($existing_researchers));

  $diff_list = '<ul>';

  foreach ($diff as $d) {
    $diff_list .= '<li>' . $d . '</li>';
  }

  $diff_list .= '</ul>';

  $form['list'] = array(
    '#value' => $diff_list,
  );

  return $form;
}

function islandora_scholar_associate_button_form(&$form_state, $pid = NULL) {

  $form['associate'] = array(
    '#type' => 'submit',
    '#value' => t('Associate'),
  );

  $form['pid'] = array(
    '#type' => 'hidden',
    '#value' => htmlentities($pid),
  );

  return $form;
}

function islandora_scholar_associate_button_form_submit($form, &$form_state) {

  $pid = $form_state['values']['pid'];

  drupal_goto('scholar_associate/' . $pid);
}

function islandora_scholar_associate_citation_form(&$form_state, $pid) {

  module_load_include('inc', 'fedora_repository', 'api/fedora_item');
  module_load_include('inc', 'scholar', 'Coins');
  module_load_include('inc', 'citeproc', 'CiteProcJSBibliography');
  module_load_include('inc', 'scholar', '/citation/bibliography/BibliographyForm');
  module_load_include('inc', 'scholar', 'UsersClass');

  $csl = 'default'; // @todo This needs to be changed to a value stored in the db

  $pid = html_entity_decode($pid);

  $form = array();

  $item = new Fedora_Item($pid);

  $mods = $item->get_datastream_dissemination('MODS');
  $mods_xml = new SimpleXMLElement($mods);

  $details = new Coins($pid);

  $solr_url = 'http://localhost:8080/solr/select?q=mads.namePart:' . implode('%20OR%20mads.namePart:', $details->family_names) . '&fl=PID,mads.fullname,mads.username,mads.topic,&rows=10000';
  $solr_query = @file_get_contents($solr_url);
  if ($solr_query != NULL) {
    $solr_xml = new SimpleXMLElement($solr_query);
    $results = $solr_xml->xpath('//doc');
    $researcher_pids = array();

    foreach ($results as $result) {
      $researcher_pid = (string) $result->str;
      $researcher = explode(':', $researcher_pid);
      $researcher_pids[$researcher[1]]['pid'] = $researcher_pid;
      $researcher_name = (string) $result->arr[0]->str;
      $researcher_pids[$researcher[1]]['name'] = $researcher_name;
      $researcher_cn = $result->xpath('./arr[@name="mads.username"]');
      var_dump($researcher_cn[0]);
      $researcher_pids[$researcher[1]]['cn'] = (string) $researcher_cn[0]->str;
      $researcher_dept = $result->xpath('./arr[@name="mads.topic"]');
      var_dump($researcher_dept);
      $researcher_pids[$researcher[1]]['dept'] = (string) $researcher_dept[0]->str;
      $researcher_keys1[$researcher[1]] = $researcher_pid;
    }

//    $list = islandora_scholar_ldap_search($details->family_names);
//
//    $count = $list['count'];
////    var_dump($list);
//    for ($i = 0; $i < $count; $i++) {
//
//      $cn = strtolower($list[$i]['cn'][0]);
//      $ldap_groups = array();
//      if (is_array($list[$i]['groupmembership'])) {
//        foreach ($list[$i]['groupmembership'] as $ldap_group) {
//          $ldap_dept = strtok($ldap_group, ',');
//          $ldap_dept = ltrim($ldap_dept, 'cn=');
//          $ldap_dept = islandora_scholar_convert_dept($ldap_dept);
//          $ldap_dept = str_replace(' ', '_', $ldap_dept);
//          if ($ldap_dept) {
//            $ldap_groups[] = $ldap_dept;
//          }
//        }
//      }
//      $ldap_groups = array_unique($ldap_groups);
//      $users[$i]['uid'] = $list[$i]['uid'][0];
//      $users[$i]['cn'] = $cn;
//      $users[$i]['fullname'] = $list[$i]['fullname'][0];
//      $users[$i]['mail'] = $list[$i]['mail'][0];
//      $users[$i]['dept'] = implode(', ', $ldap_groups);
//      $researcher_keys2['ldap:' . $cn] = 'ldap:' . $cn;
//    }

    $researcher_keys = array_merge($researcher_keys1, $researcher_keys2);
    $table = array(
      '#header' => array(theme('table_select_header_cell', array(1)), t('Full name'), t('Username'), t('Department'), t('Source'), t('Added'), ''),
      '#theme' => THEME_BIBLIOGRAPHY_FORM_TABLE,
      '#tree' => TRUE,
      'rows' => array(),
      'selections' => array(
        '#type' => 'checkboxes',
        '#options' => array_fill_keys($researcher_keys, ''),
      ),
    );
    $rows1 = &$table['rows'];

    foreach ($researcher_pids as $r_pid) {
      in_array($r_pid['cn'], $details->username) ? $user_image = '<img src="' . url('misc/watchdog-ok.png') . '"/>' : $user_image = '<img src="' . url('misc/watchdog-error.png') . '"/>';
      $rows1[] = array(
        '#pid' => $r_pid['pid'],
        'fullname' => array('#value' => check_plain($r_pid['name'])),
        'cn' => array('#value' => check_plain($r_pid['cn'])),
        'dept' => array('#value' => check_plain($r_pid['dept'])),
        'source' => array('#value' => check_plain('Authority records')),
        'image' => array('#value' => $user_image),
      );
    }
  }

  $rows2 = &$table['rows'];

  foreach ($users as $user) {

    $rows2[] = array(
      '#pid' => 'ldap:' . $user['cn'],
      'fullname' => array('#value' => check_plain($user['fullname'])),
      'cn' => array('#value' => check_plain($user['cn'])),
      'dept' => array('#value' => check_plain($user['dept'])),
      'source' => array('#value' => check_plain('LDAP')),
    );
  }

  $rows = array_merge($rows1, $rows2);
  $style = citeproc_style($csl);
  $bibliography = new CiteProcJSBibliography($style);
  $mods = islandora_bibilgraphy_form_get_mods($pid);
  if ($mods) {
    $ref = citeproc_citation_from_mods($mods);
    $bibliography->addCitation($ref);
  }
  $citation = "<h3>Citation view</h3>";
  $citation .= $bibliography->render();

  $names = $details->full_names;

  $name_list = '<ul>';

  foreach ($names as $name) {
    $name_list .= '<li>' . $name . '</li>';
  }

  $name_list .= '</ul>';

  $form['citation'] = array(
    '#value' => $citation,
  );

  $form['names'] = array(
    '#value' => $name_list,
    '#prefix' => '<h3>Author list</h3>',
  );
  
  $form['existing'] = array(
    '#value' => '<ul><li>' . implode('</li><li>', $details->username) . '</li></ul>',
    '#prefix' => '<h3>Already associated</h3>',
  );

  $form['table'] = array(
    '#prefix' => '<h3>Potential matches</h3>',
    'table' => $table,
  );

  return $form;
}

function islandora_scholar_associate_citation_form_submit($form, &$form_state) {
  
}