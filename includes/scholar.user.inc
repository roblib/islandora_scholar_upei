<?php

/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

/**
 * creates an object in fedora the pid is saved in $user->data as islandora_user_pid
 * @param object $edit
 * @return string the pid of the newly created object
 */

function scholar_create_user_object($form, $form_state) {
  //if display_name is not set we don't create an authority record in Fedora.
  if(empty($form_state['values']['display_name'])){
    drupal_set_message(t('Did not create a Fedora authority record for this user. Display name was not set'),'info');
    return;
  }
  module_load_include('inc', 'fedora_repository', 'api/fedora_item');
  $pid = 'ir:'.$form_state['values']['name'];
  $object = Fedora_Item::ingest_new_item($pid, 'A', $edit['name'] . ' Projects');
  //@TODO the values in the below should be pulled into an admin interface so this can be generalized and reused
  $object->add_relationship('isMemberOfCollection', 'ir:people', RELS_EXT_URI);
  $object->add_relationship('hasModel', 'islandora:researcherCModel', FEDORA_MODEL_URI);
  scholar_add_mads($form_state, $object);
  drupal_set_message(t('created user object %p', array('%p' => $pid)),'info');
}

/**
 * Create the MADS from info in the form and add it to the Fedora object
 * @param object $form_state 
 * @param object $object a fedora object
 */
function scholar_add_mads($form_state, $object) {
  //@TODO add form elements for further info so we can populate more of the eac-cpf record
  //@TODO will aslo have to update the eac-cpf on user update etc.
  //@TODO complete this function
  global $user;
  $mads_template = '<mads xmlns="http://www.loc.gov/mads/v2" xmlns:mads="http://www.loc.gov/mads/v2" xmlns:xlink="http://www.w3.org/1999/xlink" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:schemaLocation="http://www.loc.gov/mads/v2 http://www.loc.gov/standards/mads/mads.xsd">
  <authority>
    <name type="personal">
      <namePart type="given">given_name</namePart>
      <namePart type="family">family_name</namePart>
      <namePart type="termsOfAddress"></namePart>
      <namePart type="date"></namePart>
    </name>
    <titleInfo>
      <title>display_name</title>
    </titleInfo>
  </authority>
  <variant>
    <name>
      <namePart type="given"></namePart>
      <namePart type="family"></namePart>
    </name>
  </variant>
  <identifier type="u1">login_name</identifier>
  <affiliation>
    <email>email_address</email>
    <organization>department_value</organization>
    <position>position_name</position>
    <phone>phone_number</phone>
    <dateValid point="start">date_start</dateValid>
    <dateValid point="end">date_end</dateValid>
  </affiliation>
  <fieldOfActivity></fieldOfActivity>
  <note>note_value</note>
  <url>url_value</url>
</mads>';
  $mads_template = str_replace('family_name', check_plain($form_state['values']['family_name']), $mads_template);
  $mads_template = str_replace('given_name', check_plain($form_state['values']['given_name']), $mads_template);
  $mads_template = str_replace('login_name', $form_state['values']['name'], $mads_template);
  $mads_template = str_replace('email_address', $form_state['values']['mail'], $mads_template);
  $mads_template = str_replace('department_value', check_plain($form_state['values']['department']), $mads_template);
  $mads_template = str_replace('position_name', check_plain($form_state['values']['position']), $mads_template);
  $mads_template = str_replace('phone_number', check_plain($form_state['values']['phone']), $mads_template);
  $mads_template = str_replace('date_valid', $form_state['values']['date'], $mads_template);
  $mads_template = str_replace('note_value', check_plain($form_state['values']['notes']), $mads_template);
  $mads_template = str_replace('url_value', $form_state['values']['url'], $mads_template);
  $mads_template = str_replace('display_name', $form_state['values']['display_name'], $mads_template);
   $start_date_arr = $form_state['values']['start_date'];
  if ($start_date_arr['year'] && $start_date_arr['month'] && $start_date_arr['day']) {
    $start_date = $start_date_arr['year'].'/'.$start_date_arr['month'].'/'.$start_date_arr['day'];
  }
  else {
    $start_date = " ";
  }
 $start_date_text = $start_date;
  
  $end_date_arr = $form_state['values']['end_date'];
  if ($end_date_arr['year'] && $end_date_arr['month'] && $end_date_arr['day']) {
    $end_date = new DateTime();
    if ($end_date->format('Y') == $end_date_arr['year'] && $end_date->format('m') == $end_date_arr['month'] && $end_date->format('d') == $end_date_arr['day']) {
      $end_date = " ";
    }
    else {
      $end_date->setDate($end_date_arr['year'], $end_date_arr['month'], $end_date_arr['day']);
      $end_date = $end_date->format('Y/m/d');
    }
  }
  else {
    $end_date = " ";
  }
  $end_date_text = $end_date;
  
  $mads_template = str_replace('date_start', $start_date_text, $mads_template);
  $mads_template = str_replace('date_end', $end_date_text, $mads_template);
  $object->add_datastream_from_string($mads_template, 'MADS', 'MADS', 'text/xml', 'X', 'Added MADS');
  $object->modify_object(check_plain($form_state['values']['display_name']), NULL, $form_state['values']['name'], 'Modified by scholar module', TRUE);
}
?>
