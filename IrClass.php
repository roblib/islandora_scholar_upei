<?php

/*
 * Created on 28-Oct-08
 *
 * To change the template for this generated file go to
 * Window - Preferences - PHPeclipse - PHP - Code Templates
 */

class IrClass {

  function IrClass() {
    drupal_bootstrap(DRUPAL_BOOTSTRAP_FULL);
  }

  //gets fullname when passed a username
  //we are currently using a file to get this info
  //originally we sucked this out of the db but since most faculty have
  //never logged in we never get their info out of ldap and since there is no
  //easy way to determine if a users is faculty or staff in our ldap we are a text file list
  //to generate the dropdowns
  function get_user_by_username($username) {
    module_load_include('inc', 'Fedora_Repository', 'api/fedora_utils');
    if (!isset($username)) {
      drupal_set_message(t('No username provided!'), 'error');
    }
    $result = db_query("SELECT users.uid FROM {users} where users.name = '%s'", $username);
    $user_id = db_fetch_object($result);
    $user = user_load(array(
      'uid' => $user_id->uid
        ));
    return $user->profile_name; //.' ('.$user->name.') ';
  }

  function addStreamFormSubmit($form_id, $form_values) {
    module_load_include('inc', 'Fedora_Repository', 'api/fedora_utils');
    module_load_include('inc', 'Fedora_Repository', 'api/fedora_item');
    module_load_include('php', 'Fedora_Repository', 'ObjectHelper');

    $types = array(
      'application/vnd.openxmlformats-officedocument.wordprocessingml.document',
      'application/vnd.wordperfect',
      'application/wordperfect',
      'application/vnd.oasis.opendocument.text',
      //'application/vnd.openxmlformats-officedocument.presentationml.presentation',
      'text/rtf',
      'application/rtf',
      'application/msword',
      'application/vnd.ms-powerpoint',
      'application/pdf'
    );
    global $user;
    /* TODO Modify the validators array to suit your needs.
      This array is used in the revised file_save_upload */

    $fileObject = file_save_upload('file_uploaded');
    if (!in_array($fileObject->filemime, $types)) {
      drupal_set_message(t('The detected mimetype %s is not supported', array(
            '%s' => $fileObject->filemime
          )), 'error');
      return false;
    }
    file_move($fileObject->filepath, 0, 'FILE_EXISTS_RENAME');
    $objectHelper = new ObjectHelper();
    $pid = $form_values['pid'];
    $fedora_item = new Fedora_Item($pid);
    $test = NULL;
    $test = $fedora_item->add_datastream_from_file($fileObject->filepath, 'OBJ');
    if ($test) {
      $this->updateMODSStream($form_values['pid'], $form_values['version'], $form_values['usage']);
    }
    return true;
  }

  function updateMODSStream($pid, $version = NULL, $usage = NULL, $xmlString = NULL) {
    module_load_include('inc', 'fedora_repository', 'fedora_item');
    $object = new Fedora_Item($pid);
    if (!isset($xmlString)) {
      $mods = $object->get_datastream_dissemination('MODS');
    }
    else {
      $mods = $xmlString;
    }
    $doc = new DOMDocument('1.0', 'UTF-8');
    $doc->substituteEntities = FALSE;
    try {
      $doc->loadXML($mods);
    } catch (exception $e) {
      drupal_set_message(t('Error loading MODS record! ') . $e->getMessage());
      return ' ';
    }
    if (isset($usage)) {
      $usageNode = $doc->createElement('accessCondition', $usage);
      $usageNode->setAttribute('type', 'use and reproduction');
      $nodeList = $doc->getElementsByTagName('mods');
      foreach ($nodeList as $reference) {
        $reference->appendChild($usageNode);
      }
    }
    if (isset($version)) {
      //check if the physicalDescription element exists
      if ($doc->getElementsByTagName('physicalDescription')->length != 0) {
        drupal_set_message(t('physicalDescription element already exists!'));
        $statusNode = $doc->getElementsByTagName('physicalDescription');
        $form_node = $doc->createElement('form', $version);
        $form_node->setAttribute('authority', 'local');
        foreach ($statusNode as $node) {
          $node->appendChild($form_node);
        }
        //add it to the reference node
        $reference_node_list = $doc->getElementsByTagName('mods');
      }
      else {
        //create the useage element as it does not exist
        drupal_set_message(t('physicalDescription element does not exist, creating...'));
        $statusNode = $doc->createElement('physicalDescription');
        $form_node = $doc->createElement('form', $version);
        $form_node->setAttribute('authority', 'local');
        $statusNode->appendChild($form_node);
        //add it to the reference node
        $reference_node_list = $doc->getElementsByTagName('mods');
        foreach ($reference_node_list as $reference) {
          $reference->appendChild($statusNode);
        }
      }
    }
    $result = $object->modify_datastream_by_value($doc->saveXML(), 'MODS', 'MODS record', 'text/xml', TRUE, 'MODS datastream modified');
  }

  //parses the return from the ruleengine framework
  function parseReturnValue($input) {
    $doc = new DOMDocument();
    $doc->loadXML($input);
    $nodeList = $doc->getElementsByTagName('message');
    foreach ($nodeList as $node) {
      if ("Document conversion complete." == $node->nodeValue) {
        $input = $node->nodeValue;
        return true;
      }
    }

    $input = "Error during file conversion!";
    return false;
  }

  function build_add_datastream_form($pid) {
    $form = array();
    $certifyBox = array(
      '1' => 'I Certify That I have the right to upload this version.'
    );

    $usage = array(
     
      'Contact Publisher' => 'Contact Publisher (I do not hold the copyright)',
      'Contact Author' => 'Contact Author (I hold the copyright and wish to retain all rights)',
      'CC-BY-NC' => '<a rel="license" title="Creative Commons Attribution-NonCommercial 3.0 Unported License" 
   href="http://creativecommons.org/licenses/by-nc/3.0/">
  <img alt="Creative Commons License" style="border-width:0" src="http://i.creativecommons.org/l/by-nc/3.0/80x15.png" />
  </a> This work is licensed under a <a rel="license" 
   title="Creative Commons Attribution-NonCommercial 3.0 Unported License" 
   href="http://creativecommons.org/licenses/by-nc/3.0/">Creative Commons
   Attribution-NonCommercial 3.0 Unported License</a>',
      'CC-BY-NC-SA' => '<a rel="license" title="Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License" 
   href="http://creativecommons.org/licenses/by-nc-sa/3.0/">
  <img alt="Creative Commons License" style="border-width:0" src="http://i.creativecommons.org/l/by-nc-sa/3.0/80x15.png" />
  </a> This work is licensed under a <a rel="license" 
   title="Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License" 
   href="http://creativecommons.org/licenses/by-nc-sa/3.0/">Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License</a>',
      'CC-BY-NC-ND' => '<a rel="license" title="Creative Commons Attribution-NonCommercial-NoDerivatives 3.0 Unported License" 
   href="http://creativecommons.org/licenses/by-nc-nd/3.0/">
  <img alt="Creative Commons License" style="border-width:0" src="http://i.creativecommons.org/l/by-nc-nd/3.0/80x15.png" />
  </a> This work is licensed under a <a rel="license" 
   title="Creative Commons Attribution-NonCommercial-NoDerivatives 3.0 Unported License" 
   href="http://creativecommons.org/licenses/by-nc-nd/3.0/">Creative Commons Attribution-NonCommercial-NoDerivatives 3.0 Unported License</a>'
    );

    $versions = array(
      'PRE-PRINT' => 'PRE-PRINT',
      'POST-PRINT' => 'POST-PRINT',
      'PUBLISHED' => 'PUBLISHED PDF',
      'OTHER' => 'OTHER/UNPUBLISHED',
    );

    $form['#attributes'] = array('enctype' => "multipart/form-data");

    $form['file_uploaded'] = array(
      '#type' => 'file',
      '#title' => t('Upload Document'),
      '#size' => 48,
      '#weight' => 1,
      '#description' => t('Full text'),
    );

    $form['version'] = array(
      '#type' => 'radios',
      '#title' => t('Document Version'),
      '#options' => $versions,
      '#required' => 'true',
      '#weight' => 2,
    );

    $form['usage'] = array(
      '#type' => 'radios',
      '#title' => t('Use Permission'),
      '#options' => $usage,
      '#required' => 'true',
      '#default_value' => 1,
      '#weight' => 3
    );

    $form['certify'] = array(
      '#type' => 'checkboxes',
      '#title' => t('I Certify'),
      '#options' => $certifyBox,
      '#required' => 'true',
      '#weight' => 4,
    );

    $form['pid'] = array(
      '#type' => 'hidden',
      '#value' => $pid,
    );

    $form['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Upload'),
      '#weight' => 5,
      '#suffix' => t('Please be patient. Document upload and conversion can take up to a few minutes.'),
    );

    return $form;
  }

}