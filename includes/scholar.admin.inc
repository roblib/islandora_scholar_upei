<?php

/**
 * @file
 */
function scholar_admin_settings() {
  module_load_include('inc', 'scholar', 'citation/csl/CSL');
  $form = array();
  $form['collection'] = array(
    '#type' => 'fieldset',
    '#title' => t('Collection'),
  );

  $form['collection']['islandora_scholar_new_citation_collection'] = array(
    '#type' => 'textfield',
    '#title' => t('Collection PID'),
    '#description' => t('Identifier of collection to hold newly harvested, unedited citation objects.'),
    '#default_value' => variable_get('islandora_scholar_new_citation_collection', 'ir:collection'),
  );
  $form['romeo'] = array(
    '#type' => 'fieldset',
    '#title' => t('RoMEO settings'),
  );

  $form['romeo']['scholar_romeo_check'] = array(
    '#type' => 'checkbox',
    '#title' => 'Display RoMEO information',
    '#default_value' => variable_get('scholar_romeo_check', 1),
  );

  $form['romeo']['scholar_romeo_url'] = array(
    '#type' => 'textfield',
    '#title' => 'RoMEO API URL',
    '#default_value' => variable_get('scholar_romeo_url', 'http://www.sherpa.ac.uk/romeo/api29.php?issn='),
    '#size' => 50,
  );

  $form['romeo']['scholar_romeo_key'] = array(
    '#type' => 'textfield',
    '#title' => 'RoMEO API Key',
    '#default_value' => variable_get('scholar_romeo_key', ''),
    '#size' => 20,
  );

  $form['romeo']['scholar_romeo_cache_time'] = array(
    '#type' => 'textfield',
    '#title' => 'RoMEO Content Cache Time',
    '#description' => t('Time, in days, to store cached RoMEO data'),
    '#default_value' => variable_get('scholar_romeo_cache_time', '2'),
    '#size' => 3,
  );
  $form['doi'] = array(
    '#type' => 'fieldset',
    '#title' => t('DOI configuration'),
  );

  $form['doi']['doi_importer_openurl'] = array(
    '#type' => 'textfield',
    '#title' => t('DOI OpenURL'),
    '#description' => t('The URL which we will query to obtain our CrossREF information.'),
    '#default_value' => variable_get('doi_importer_openurl', 'http://www.crossref.org/openurl'),
  );

  $form['doi']['doi_importer_openurl_pid'] = array(
    '#type' => 'textfield',
    '#title' => t('OpenURL PID'),
    '#description' => t('An identifier to call yourself, for the OpenURL endpoint. To use this service you first need to register for an account here: http://www.crossref.org/requestaccount/ '),
    '#default_value' => variable_get('doi_importer_openurl_pid', 'user@example.com'),
  );


  $form['ldap'] = array(
    '#type' => 'fieldset',
    '#title' => t('LDAP settings'),
  );

  $form['ldap']['scholar_ldap_host'] = array(
    '#type' => 'textfield',
    '#title' => t('Host'),
    '#default_value' => variable_get('scholar_ldap_host', ''),
  );

  $form['ldap']['scholar_ldap_port'] = array(
    '#type' => 'textfield',
    '#title' => t('Port'),
    '#default_value' => variable_get('scholar_ldap_port', '389'),
  );

  $form['ldap']['scholar_ldap_basedn'] = array(
    '#type' => 'textfield',
    '#title' => t('Base DN'),
    '#default_value' => variable_get('scholar_ldap_basedn', ''),
  );

  $form['ldap']['scholar_ldap_bind_user'] = array(
    '#type' => 'textfield',
    '#title' => t('Bind user'),
    '#default_value' => variable_get('scholar_ldap_bind_user', ''),
  );

  $form['ldap']['scholar_ldap_bind_pass'] = array(
    '#type' => 'textfield',
    '#title' => t('Bind password'),
    '#default_value' => variable_get('scholar_ldap_bind_pass', ''),
  );


  $form['scholar_cache'] = array(
    '#type' => 'fieldset',
    '#title' => t('Cache settings'),
  );

  $form['scholar_cache']['scholar_researcher_citations_cache_time'] = array(
    '#type' => 'textfield',
    '#title' => t('Researcher citations cache time in days'),
    '#default_value' => variable_get('scholar_researcher_citations_cache_time', '2'),
  );

  $form['scholar_csl'] = array(
    '#type' => 'fieldset',
    '#title' => t('CSL settings'),
  );

  $csl = new CSL();

  $options = $csl->GetNames();

  $form['scholar_csl']['scholar_default_csl'] = array(
    '#type' => 'select',
    '#options' => $options,
    '#default_value' => variable_get('scholar_default_csl', 'default'),
  );

  $form['scholar_solr'] = array(
    '#type' => 'fieldset',
    '#title' => t('Solr settings'),
  );

  $form['scholar_solr']['scholar_solr_node_for_header'] = array(
    '#type' => 'textarea',
    '#title' => t('Nodes and Values for search header config'),
    '#default_value' => variable_get('scholar_solr_node_for_header', 'Institutional Repository Collection ~ 8'),
    '#description' => 'Enter a search filter and a node id ie. Institutional Repository Collection ~ 8 each entry should be on its own line'
  );

  return system_settings_form($form);
}

function scholar_admin_settings_validate($form, $form_state) {
  if (!isset($form_state['values']['scholar_romeo_url'])) {
    form_set_error('url', t('URL cannot be empty!'));
  }
  if (!isset($form_state['values']['scholar_romeo_key'])) {
    form_set_error('key', t('Key cannot be empty!'));
  }
}