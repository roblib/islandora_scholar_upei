<?php

/**
 * @file
 */

function scholar_admin_settings() {
  $form = array();
  
  $form['romeo'] = array(
    '#type' => 'fieldset',
    '#title' => t('RoMEO settings'),
  );
  
  $form['romeo']['scholar_romeo_url'] = array(
    '#type' => 'textfield',
    '#title' => 'RoMEO API URL',
    '#default_value' => variable_get('scholar_romeo_url', ''),
    '#size' => 50,
  );
  
  $form['romeo']['scholar_romeo_key'] = array(
    '#type' => 'textfield',
    '#title' => 'RoMEO API Key',
    '#default_value' => variable_get('scholar_romeo_key', ''),
    '#size' => 20,
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
  
  $form['jodconverter'] = array(
    '#type' => 'fieldset',
    '#title' => t('JODConverter settings'),
  );
  
  $form['jodconverter']['scholar_jod_path'] = array(
    '#type' => 'textfield',
    '#title' => t('JODConverter path'),
    '#default_value' => variable_get('scholar_jod_path', 'localhost:8080/RuleEngineServlet/RuleEngine'),
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