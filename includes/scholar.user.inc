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
function scholar_create_user_object(&$edit, &$account) {
  module_load_include('inc', 'fedora_repository', 'api/fedora_item');
  $pid = Fedora_Item::get_next_PID_in_namespace('si-user');
  $object = Fedora_Item::ingest_new_item($pid, 'A', $edit['name'] . ' Projects');
  //@TODO the values in the below should be pulled into an admin interface so this can be generalized and reused
  $object->add_relationship('isMemberOfCollection', 'ir:people', RELS_EXT_URI);
  $object->add_relationship('hasModel', 'islandora:researcherCModel', FEDORA_MODEL_URI);
  //$path_to_policy = drupal_get_path('module', 'sidora') . '/data/policies/default_COLLECTION_POLICY.xml';
  //$object->add_datastream_from_file($path_to_policy, 'COLLECTION_POLICY', 'COLLECTION_POLICY', 'text/xml', 'M', 'Sidora added Collection Policy Stream');
  //sidora_update_object_xacml($object,sidora_create_default_xacml($edit['name']));
  scholar_add_mads($edit, $object);
  $edit['islandora_user_pid'] = $pid;
  return $pid;
}

/**
 * Create the EAC-CPF from info in the edit object and add it to the Fedora object
 * @param object $edit a drupal user object
 * @param object $object a fedora object
 */
function scholar_add_maps($edit, $object) {
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
    <dateValid>date_valid</dateValid>
  </affiliation>
  <fieldOfActivity></fieldOfActivity>
  <note></note>
  <url></url>
</mads>';
  $mads_template = str_replace('user_name', $user->name, $mads_template);
  $mads_template = str_replace('given_name', $user->name, $mads_template);
  $mads_template = str_replace('account_name', $edit['name'], $mads_template);
  $mads_template = str_replace('date_now', date("c"), $mads_template);
  $mads_template = str_replace('record_id', $object->pid, $mads_template);
  $object->add_datastream_from_string($eac_cpf_template, 'MADS', 'MADS', 'text/xml', 'M', 'Added MADS');
}
?>
