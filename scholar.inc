<?php

/**
 * @file
 *
 * Provides the functions to process and view chemical structure files.
 */
class Scholar {

  function __construct($pid = '') {
    module_load_include('inc', 'fedora_repository', 'api/fedora_item');
    if (!empty($pid)) {
      $this->pid = $pid;
      $this->item = new Fedora_Item($pid);
    }
  }

  function showCitation() {
//    module_load_include('inc', 'scholar', 'scholar_tracking');
    $pid = $this->pid;
//    $track = new islandora_scholar_tracking($pid);
//    $track->add_page_count();
//    $read = $track->read_page_count();
    $tabset = islandora_scholar_build_citation($pid);
//    $tabset['views'] = array(
//      '#value' => '<div>Times viewed: ' . $read['count'] . '</div>',
//    );
////    var_dump($read);
//    $tabset['last_viewed'] = array(
//      '#value' => '<div>Last viewed: ' . $read[($read['count'] - 1)]['time'] . '</div>',
//    );
    return $tabset;
  }

  function showThesis() {
//   Edited by rwincewicz on 25 Oct 2011
//   ROMEO changed the URL they were using and so we had to update our code. Before the URL was stored as an external
//   datastream in Fedora and so would require every object to have the ROMEO datastream edited. This code now brings
//   the xml from the ROMEO API using a URL that is created dynamically from the code below. Redmine #5615
//   URL and API key for the ROMEO REST client. TODO: Add this to the admin panel so that users can change it easily.
//
//    global $base_url;
//    global $base_path;
//    global $user;
//    module_load_include('inc', 'fedora_repository', 'ObjectHelper');
//    module_load_include('inc', 'fedora_repository', 'api/fedora_item');
//    module_load_include('inc', 'citeproc', 'CiteProcJSBibliography');
//    module_load_include('inc', 'scholar', '/citation/bibliography/BibliographyForm');
//    module_load_include('php', 'scholar', 'IrClass');
//    module_load_include('inc', 'scholar', '/citation/bibliography/CitationForm');
//    drupal_add_css(drupal_get_path('module', 'islandora_bibliography') . '/css/export.css');
//    drupal_add_css(drupal_get_path('module', 'scholar') . '/scholar.css');
//
//    $fedora_url = variable_get('fedora_base_url', 'http://localhost:8080/fedora');
//
//    $pid = $this->pid;
//
//    $server_ip = $_SERVER['SERVER_ADDR'];
//    
//    $tabset['first_tab'] = array(
//      '#type' => 'tabpage',
//      '#title' => t('Description'),
//    );
//
//    $tabset['first_tab']['tabs'] = array(
//      '#type' => 'tabset',
//    );
//
//    $url = variable_get('scholar_romeo_url', 'http://www.sherpa.ac.uk/romeo/api29.php?issn=');
//    $api_key = '&ak=' . variable_get('scholar_romeo_key', '');
//
//    $path = drupal_get_path('module', 'Fedora_Repository');
//    module_load_include('inc', 'Fedora_Repository', 'CollectionClass');
//    module_load_include('inc', 'scholar', 'Coins');
//    $data = new Coins($this->pid);
////    Get MODS datastream and pull the ISSN from there.
//
//    $csl = 'default'; // This needs to be changed to a value stored in the db
//
//    $style = citeproc_style($csl);
//    $bibliography = new CiteProcJSBibliography($style);
//    $mods = islandora_bibilgraphy_form_get_mods($this->pid);
//    if ($mods) {
//      $ref = citeproc_citation_from_mods($mods);
//      $bibliography->addCitation($ref);
//    }
//    $citation = $bibliography->render();
//    $abstract = (string) $data->abstract;
//    if ($abstract != NULL || $abstract != '') {
//      $citation .= '<p><b>Abstract</b></p>';
//      $citation .= $data->abstract . '</br>';
//    }
//    
//    $item = new Fedora_Item($pid);
//    $ds_array = $item->get_datastreams_list_as_array();
//    
//    if (array_key_exists('ZIP', $ds_array)) {
//      $citation .= '</br><b>Supplementary information</b></br>';
//      $description = $ds_array['ZIP']['label'];
//      $citation .= '<p>' . l($description, 'fedora/repository/' . $pid . '/ZIP/ZIP') . '</p>';
//    }
//    
//    module_load_include('inc', 'islandora_bibliography', 'Bibliography');
//    
//    $refworks_link = '<A href="http://www.refworks.com/express/ExpressImport.asp?vendor=Robertson+Library&filter=RIS%20Format&url=http%3A//' . $server_ip . '/refworks_entry/' . urlencode($this->pid) . '" target="RefWorksMain">Export to RefWorks</A>';
//    
//    $citation .= '<div class="scholar_bibliography_add_form">' . drupal_get_form('islandora_bibliography_citation_form', $pid) . '</div>';
//
//    $citation .= '<div class="scholar_refworks_link">' . $refworks_link . '</div>';
//
//    //    Get XML directly from ROMEO building the URL from the ISSN found in the Refworks DS.
//
//    $issn_url = file_get_contents($url . $data->issn . $api_key);
//    try {
//      $proc = new XsltProcessor();
//    } catch (Exception $e) {
//      drupal_set_message(t('@error', array('@error' => $e->getMessage())), 'error');
//      return;
//    }
//    $xsl = new DOMDocument();
//    $xsl->load($path . '/xsl/romeo.xsl');
//    $input = new DOMDocument();
//    $input->loadXML(trim($issn_url));
//    $xsl = $proc->importStylesheet($xsl);
//    $newdom = $proc->transformToDoc($input);
//    $romeo = $newdom->saveXML();
//  
//    $coins_title = '&#10;&#9;&#9;&#9;&#9;ctx_ver=Z39.88-2004&amp;rft_val_fmt=info%3Aofi%2Ffmt%3Akev%3Amtx%3Ajournal&amp;rfr_id=info:sid/library.upei.ca:Robertson&amp;rft.issn=' . $data->issn . '&amp;rft.date=' . $data->date . '&amp;rft.volume=' . $data->volume . '&amp;rft.issue=' . $data->issue . '&amp;rft.spage=' . $data->start_page . '&amp;rft.atitle=' . $data->title . '&amp;rft.jtitle=' . $data->journal_title;
//
//    $coins_url = '&#10;&#9;&#9;&#9;&#9;http://articles.library.upei.ca:7888/godot/hold_tab.cgi?hold_tab_branch=PCU&amp;issn=' . $data->issn . '&date=' . $data->date . '&volume=' . $data->volume . '&issue=' . $data->issue . '&spage=' . $data->start_page . '&atitle=' . $data->title . '&stitle=' . $data->sub_title;
//
//    $citation .= '<div class="scholar_coins_title"><span>  <br /><span class="Z3988" title="' . $coins_title . '"></span></div><div class="scholar_coins_url"><br/><A HREF="' . $coins_url . '"><img src="http://library.upei.ca/sites/all/files/sharedfolder/files/getit_upei.gif" /> </A></span><br /></div>';
//
//    $tabset['first_tab']['tabs']['summary'] = array(
//      '#type' => 'tabpage',
//      '#title' => t('Citation'),
//      '#content' => $citation,
//      '#weight' => 1,
//    );
//
//    $tabset['first_tab']['tabs']['romeo'] = array(
//      '#type' => 'tabpage',
//      '#title' => t('Romeo'),
//      '#content' => $romeo,
//      '#weight' => 2,
//    );
//
//    if (user_access('add scholarly research') == TRUE && array_key_exists('PDF', $ds_array) == FALSE) {
//
//      $tabset['first_tab']['tabs']['upload'] = array(
//        '#type' => 'tabpage',
//        '#title' => t('Upload'),
//        '#content' => drupal_get_form('scholar_add_ir_datastream_form', $pid),
//        '#weight' => 3,
//      );
//    }
//    else {
//      if (array_key_exists('PDF', $ds_array)) {
//        $tabset['first_tab']['tabs']['pdf'] = array(
//          '#type' => 'tabpage',
//          '#title' => t('PDF'),
//          '#content' => l(t('View PDF'), 'fedora/repository/' . $pid . '/PDF/PDF'),
//          '#weight' => 3,
//        );
//        if (array_key_exists('SIGNATURE', $ds_array) && array_key_exists('PDF', $ds_array)) {
//          $tabset['first_tab']['tabs']['signature'] = array(
//          '#type' => 'tabpage',
//          '#title' => t('Signature'),
//          '#content' => l(t('Signature file'), 'fedora/repository/' . $pid . '/SIGNATURE/SIGNATURE'),
//          '#weight' => 4,
//        );
//        }
//        else {
//        $tabset['first_tab']['tabs']['signature'] = array(
//          '#type' => 'tabpage',
//          '#title' => t('Signature'),
//          '#content' => drupal_get_form('islandora_scholar_signature_form', $pid),
//          '#weight' => 4,
//        );
//        }
//      }
//      else {
//        $tabset['first_tab']['tabs']['upload'] = array(
//          '#type' => 'tabpage',
//          '#title' => t('PDF'),
//          '#content' => t('No PDF has been uploaded yet'),
//          '#weight' => 3,
//        );
//      }
//    }
//
//    if (array_key_exists('ZIP', $ds_array)) {
//    
//    $tabset['first_tab']['tabs']['supplementary'] = array(
//        '#type' => 'tabpage',
//        '#title' => t('Supplementary information'),
//        '#content' => t('Supplementary information'),
//        '#weight' => 5,
//      );
//    }
//    else {
//      if (user_access('add scholarly research')) {
//        $tabset['first_tab']['tabs']['supplementary'] = array(
//        '#type' => 'tabpage',
//        '#title' => t('Supplementary information'),
//        '#content' => drupal_get_form('islandora_scholar_supp_info_form', $pid),
//        '#weight' => 5,
//      );
//      }
//    }
//    
//    return $tabset;

    $pid = $this->pid;
    $item = new Fedora_Item($pid);
    $ds_array = $item->get_datastreams_list_as_array();
    $tabset = islandora_scholar_build_citation($pid);

    if (array_key_exists('SIGNATURE', $ds_array) && array_key_exists('PDF', $ds_array)) {
      $tabset['first_tab']['tabs']['signature'] = array(
        '#type' => 'tabpage',
        '#title' => t('Signature'),
        '#content' => l(t('Signature file'), 'fedora/repository/' . $pid . '/SIGNATURE/SIGNATURE'),
        '#weight' => 4,
      );
    }
    else {
      $tabset['first_tab']['tabs']['signature'] = array(
        '#type' => 'tabpage',
        '#title' => t('Signature'),
        '#content' => drupal_get_form('islandora_scholar_signature_form', $pid),
        '#weight' => 4,
      );
    }

    return $tabset;
  }

  function show_researcher() {
    global $base_url;
    global $base_path;
    global $user;
    module_load_include('inc', 'fedora_repository', 'api/fedora_item');
    module_load_include('inc', 'scholar', 'scholar_tracking');

    $path = drupal_get_path('module', 'scholar');

    $tabset['first_tab'] = array(
      '#type' => 'tabpage',
      '#title' => t('Description'),
    );

    $tabset['first_tab']['tabs'] = array(
      '#type' => 'tabset',
    );

    $tabset['first_tab']['tabs']['tn'] = array(
      '#value' => '<img src="' . $base_url . '/' . $path . '/images/gnome_stock_person.png"/>',
    );

    $collectionHelper = new CollectionClass();
    $xmlstr = $collectionHelper->getStream($this->pid, "MADS");
    $xml = new SimpleXMLElement($xmlstr);

    $names = $xml->authority->name->namePart;
    $fullName = $names[0] . ' ' . $names[1];

    foreach ($xml->related->topic as $dept) {
      $depts[] = str_replace('_', ' ', (string) $dept);
    }

    if (is_array($depts)) {
      $dept = implode(', ', array_unique($depts));
    }
    else {
      $dept = $depts;
    }

    $username = $xml->identifier;
    $email = $xml->affiliation->email;

    $header = array();

    $rows = array(
      array('Name', $fullName),
      array('User name', $username),
      array('Email', $email),
      array('Department(s)', $dept),
    );

    $tabset['first_tab']['tabs']['table'] = array(
      '#value' => theme('table', $header, $rows),
    );

    $username_query = file_get_contents('http://localhost:8080/solr/select/?q=mods.username:' . (str_replace(';', ' ', (strtolower($username)))) . '&fl=PID');
//    var_dump($username_query);
    $xml = new SimpleXMLElement($username_query);
    $array = $xml->xpath('//doc');
//    var_dump($array);

    $array = array_filter($array);
    foreach ($array as $arr) {
      $pid = (string) $arr->str;
      var_dump($pid);
      $object = new CollectionClass();
      $pid_xml_str = $object->getStream($pid, "MODS");
      $pid_xml = new SimpleXMLElement($pid_xml_str);
      $title = $pid_xml->titleInfo->title;
      var_dump($title);
    }

//    $tracking = new islandora_scholar_tracking($pid);
//    
//    $citations = 

    $tabset['first_tab']['tabs']['citations'] = array(
      '#value' => $citations,
    );

    return $tabset;
  }

  function show_department() {
    global $base_url;
    global $base_path;
    global $user;
    module_load_include('inc', 'fedora_repository', 'api/fedora_item');

    $path = drupal_get_path('module', 'scholar');

    $tabset['first_tab'] = array(
      '#type' => 'tabpage',
      '#title' => t('Description'),
    );

    $tabset['first_tab']['tabs'] = array(
      '#type' => 'tabset',
    );

    $tabset['first_tab']['tabs']['tn'] = array(
      '#value' => '<img src="' . $base_url . '/' . $path . '/images/gnome_stock_person.png"/>',
    );

    $collectionHelper = new CollectionClass();
    $xmlstr = $collectionHelper->getStream($this->pid, "MADS");
    $xml = new SimpleXMLElement($xmlstr);

    $names = $xml->authority->name->namePart;
    $date = $xml->authority->name->namePart[1];
    $faculty = $xml->related->name->namePart;
    $note = $xml->note;
    $url = $xml->url;

    $header = array();

    $rows = array(
      array('Name', $names),
      array('Date', $date),
      array('Parent school/faculty', $faculty),
      array('Historical note', $note),
      array('URL', $url),
    );

    $tabset['first_tab']['tabs']['table'] = array(
      '#value' => theme('table', $header, $rows),
    );

    return $tabset;
  }

}

function build_add_datastream_form(&$form_state, $pid) {
  $form = array();
  $certifyBox = array(
    '1' => 'I Certify That I have the right to upload this version.'
  );

  $usage = array(
    'Contact Publisher' => 'Contact Publisher (I do not hold the copyright)',
    'Contact Author' => 'Contact Author (I hold the copyright and wish to retain all rights)',
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

function islandora_scholar_supp_info_form(&$form_state, $pid) {
  $form = array();

  $form['#attributes'] = array('enctype' => "multipart/form-data");

  $form['upload'] = array(
    '#type' => 'file',
    '#title' => t('Attach supplementary information'),
  );

//  $form['description'] = array(
//    '#type' => 'textarea',
//    '#title' => t('Description'),
//    '#size' => 100,
//  );

  $form['supp_submit'] = array(
    '#type' => 'submit',
    '#value' => t('Submit'),
  );

  $form['pid'] = array(
    '#type' => 'value',
    '#value' => $pid,
  );

  return $form;
}

function islandora_scholar_supp_info_form_validate($form, &$form_state) {

  $fieldName = 'upload';

  if (isset($_FILES['files']) && is_uploaded_file($_FILES['files']['tmp_name'][$fieldName])) {

    $file = file_save_upload($fieldName);

    if (!$file) {
      form_set_error($fieldName, 'Error uploading file.');
      return;
    }

    $form_state['values']['file'] = $file;
  }
  else {
    form_set_error($fieldName, 'Error uploading file.');
    return;
  }
  if (file_get_mimetype($file->filepath) !== 'application/zip') {
    form_set_error($fieldName, 'This file is not a zip!');
    return;
  }
}

function islandora_scholar_supp_info_form_submit($form, &$form_state) {
  module_load_include('inc', 'fedora_repository', 'api/fedora_item');
//  drupal_set_message('Pid: ' . $form_state['values']['pid']);
  $pid = $form_state['values']['pid'];
  $file = $form_state['values']['file'];
  $name = str_replace(' ', '_', $file->filename);
  $object = new Fedora_Item($pid);
//  drupal_set_message('File: ' . $file->filepath);
  $object->add_datastream_from_file($file->filepath, 'ZIP', $name);
  $list = islandora_scholar_zip_list($file->filepath);
//  drupal_set_message('Files: ' . implode(', ', $list));
  $collectionHelper = new CollectionClass();
  $xmlstr = $collectionHelper->getStream($pid, "MODS", 0);

  $mods_xml = new DOMDocument();
  $mods_xml->loadXML($xmlstr);
  $xpath = new DOMXPath($mods_xml);
  $test = $xpath->query("//*[@type='supplement']")->item(0)->nodeValue;
//  drupal_set_message('Test: ' . $test);
  if ($test !== '') {
    $note = $mods_xml->createElement('note', implode(', ', $list));
    $note->setAttribute('type', 'supplement');
    $mods_element = $mods_xml->getElementsByTagName('mods')->item(0);
    $mods_element->appendChild($note);
//  drupal_set_message('DS: ' . $mods_xml->saveXML());
    $object->modify_datastream_by_value($mods_xml->saveXML(), 'MODS', 'MODS Record', 'text/xml');
  }
  else {
    drupal_set_message(t('Entry for supplementary information already exists!'), 'warning');
  }
}

function islandora_scholar_supp_info_delete_form(&$form_state, $pid) {
  $form = array();

  $form['supp_delete'] = array(
    '#type' => 'submit',
    '#value' => t('Delete supplemental information'),
  );

  $form['pid'] = array(
    '#type' => 'value',
    '#value' => $pid,
  );

  return $form;
}

function islandora_scholar_supp_info_delete_form_submit($form, &$form_state) {
  module_load_include('inc', 'fedora_repository', 'api/fedora_item');
  $pid = $form_state['values']['pid'];
  $object = new Fedora_Item($pid);
  $object->purge_datastream('ZIP');
  $collectionHelper = new CollectionClass();
  $xmlstr = $collectionHelper->getStream($pid, "MODS", 0);

  $mods_xml = new DOMDocument();
  $mods_xml->loadXML($xmlstr);
  $list = $mods_xml->getElementsByTagName('note');
  var_dump($list);
  $remove_nodes = array();
  foreach ($list as $element) {
    $attribute = $element->getAttribute('type');
    var_dump($attribute);
    var_dump($element);
    if ($attribute == 'supplement') {
      $remove_nodes[] = $element;
    }
  }
  var_dump($remove_nodes);
  if (!empty($remove_nodes)) {
    foreach ($remove_nodes as $remove_node) {
      $remove_node->parentNode->removeChild($remove_node);
    }
    $object->modify_datastream_by_value($mods_xml->saveXML(), 'MODS', 'MODS Record', 'text/xml');
  }
  else {
    drupal_set_message(t('No MODS element found to remove!'));
  }
}

function islandora_scholar_zip_list($zip_file) {
  exec('unzip -l "' . $zip_file . '"', $info);
  return $info;
}

function islandora_scholar_signature_form(&$form_state, $pid) {
  $form = array();

  $form['#attributes'] = array('enctype' => "multipart/form-data");

  $form['upload'] = array(
    '#type' => 'file',
    '#title' => t('Attach signature page'),
  );

  $form['supp_submit'] = array(
    '#type' => 'submit',
    '#value' => t('Submit'),
  );

  $form['pid'] = array(
    '#type' => 'value',
    '#value' => $pid,
  );

  return $form;
}

function islandora_scholar_signature_form_validate($form, &$form_state) {

  $fieldName = 'upload';

  if (isset($_FILES['files']) && is_uploaded_file($_FILES['files']['tmp_name'][$fieldName])) {

    $file = file_save_upload($fieldName);

    if (!$file) {
      form_set_error($fieldName, 'Error uploading file.');
      return;
    }

    $form_state['values']['file'] = $file;
  }
  else {
    form_set_error($fieldName, 'Error uploading file.');
    return;
  }
}

function islandora_scholar_signature_form_submit($form, &$form_state) {
  module_load_include('inc', 'fedora_repository', 'api/fedora_item');
  drupal_set_message('Pid: ' . $form_state['values']['pid']);
  $pid = $form_state['values']['pid'];
  $file = $form_state['values']['file'];
  $object = new Fedora_Item($pid);
  drupal_set_message('File: ' . $file->filepath);
  $object->add_datastream_from_file($file->filepath, 'SIGNATURE', 'SIGNATURE');
}

function islandora_scholar_build_citation($pid) {

  global $base_url;
  global $base_path;
  global $user;
  module_load_include('inc', 'fedora_repository', 'ObjectHelper');
  module_load_include('inc', 'fedora_repository', 'api/fedora_item');
  module_load_include('inc', 'citeproc', 'CiteProcJSBibliography');
  module_load_include('inc', 'scholar', '/citation/bibliography/BibliographyForm');
  module_load_include('php', 'scholar', 'IrClass');
  module_load_include('inc', 'scholar', '/citation/bibliography/CitationForm');
  module_load_include('inc', 'scholar', 'scholar_tracking');
  module_load_include('inc', 'Fedora_Repository', 'CollectionClass');
  module_load_include('inc', 'scholar', 'Coins');
  module_load_include('inc', 'islandora_bibliography', 'Bibliography');
  drupal_add_css(drupal_get_path('module', 'islandora_bibliography') . '/css/export.css');
  drupal_add_css(drupal_get_path('module', 'scholar') . '/scholar.css');

  $fedora_url = variable_get('fedora_base_url', 'http://localhost:8080/fedora');

  $server_ip = $_SERVER['SERVER_ADDR'];

  $tabset['first_tab'] = array(
    '#type' => 'tabpage',
    '#title' => t('Description'),
  );

  $tabset['first_tab']['tabs'] = array(
    '#type' => 'tabset',
  );

  $url = variable_get('scholar_romeo_url', 'http://www.sherpa.ac.uk/romeo/api29.php?issn=');
  $api_key = '&ak=' . variable_get('scholar_romeo_key', '');

  $path = drupal_get_path('module', 'Fedora_Repository');
  $data = new Coins($pid);
//    Get MODS datastream and pull the ISSN from there.

  $csl = 'default'; // This needs to be changed to a value stored in the db

  $style = citeproc_style($csl);
  $bibliography = new CiteProcJSBibliography($style);
  $mods = islandora_bibilgraphy_form_get_mods($pid);
  if ($mods) {
    $ref = citeproc_citation_from_mods($mods);
    $bibliography->addCitation($ref);
  }
  $citation = $bibliography->render();
  $abstract = (string) $data->abstract;
  if ($abstract != NULL || $abstract != '') {
    $citation .= '<p><b>Abstract</b></p>';
    $citation .= $data->abstract . '</br>';
  }

  $item = new Fedora_Item($pid);
  $ds_array = $item->get_datastreams_list_as_array();

  if (array_key_exists('ZIP', $ds_array)) {
    $ds_info = $item->get_datastream_info('ZIP');
    $citation .= '</br><b>Supplementary information</b></br>';
    $description = $ds_array['ZIP']['label'];
    $citation .= '<p>' . 'Name: ' . l($description, 'fedora/repository/' . $pid . '/ZIP/ZIP') . '</br>';
    $citation .= t('Size: @size bytes', array('@size' => $ds_info->datastream->size)) . '</br>';

    $supp_array = islandora_scholar_zip_mods_to_array($mods);

    if ($supp_array) {

      $citation .= t('Total: @files', array('@files' => $supp_array['files']));

      $header = array('Filename', 'Date', 'Size (bytes)');
      $rows = array();
      for ($i = 0; $i < count($supp_array); $i++) {
        $rows[] = $supp_array[$i];
      }

      $supp_table = theme('table', $header, $rows);

      $supp_content = $supp_table;

      $supp_fieldset['fieldset'] = array(
        '#type' => 'fieldset',
        '#title' => t('Details'),
        '#collapsible' => TRUE,
        '#collapsed' => TRUE,
      );
      $supp_fieldset['fieldset']['content'] = array(
        '#value' => $supp_content,
      );

      $citation .= drupal_render($supp_fieldset);
    }
    else {
      drupal_set_message(t('A supplementary information file is present but there is no record of its details, maybe something went wrong during upload'), 'warning');
    }
  }

  $track = new islandora_scholar_tracking($pid);
  $track->add_page_count();
  $read = $track->read_page_count();
  $citation .= '</br><div>Times viewed: ' . $read['count'] . '</div>';
  $citation .= '<div>Last viewed: ' . $read[($read['count'] - 1)]['time'] . '</div></br>';

  $refworks_link = '<A href="http://www.refworks.com/express/ExpressImport.asp?vendor=Robertson+Library&filter=RIS%20Format&url=http%3A//' . $server_ip . '/refworks_entry/' . urlencode($pid) . '" target="RefWorksMain">Export to RefWorks</A>';

  $citation .= '<div class="scholar_bibliography_add_form">' . drupal_get_form('islandora_bibliography_citation_form', $pid) . '</div>';

  $citation .= '<div class="scholar_refworks_link">' . $refworks_link . '</div>';

  //    Get XML directly from ROMEO building the URL from the ISSN found in the Refworks DS.

  $issn_url = file_get_contents($url . $data->issn . $api_key);
  try {
    $proc = new XsltProcessor();
  } catch (Exception $e) {
    drupal_set_message(t('@error', array('@error' => $e->getMessage())), 'error');
    return;
  }
  $xsl = new DOMDocument();
  $xsl->load($path . '/xsl/romeo.xsl');
  $input = new DOMDocument();
  $input->loadXML(trim($issn_url));
  $xsl = $proc->importStylesheet($xsl);
  $newdom = $proc->transformToDoc($input);
  $romeo = $newdom->saveXML();

  $coins_title = '&#10;&#9;&#9;&#9;&#9;ctx_ver=Z39.88-2004&amp;rft_val_fmt=info%3Aofi%2Ffmt%3Akev%3Amtx%3Ajournal&amp;rfr_id=info:sid/library.upei.ca:Robertson&amp;rft.issn=' . $data->issn . '&amp;rft.date=' . $data->date . '&amp;rft.volume=' . $data->volume . '&amp;rft.issue=' . $data->issue . '&amp;rft.spage=' . $data->start_page . '&amp;rft.atitle=' . $data->title . '&amp;rft.jtitle=' . $data->journal_title;

  $coins_url = '&#10;&#9;&#9;&#9;&#9;http://articles.library.upei.ca:7888/godot/hold_tab.cgi?hold_tab_branch=PCU&amp;issn=' . $data->issn . '&date=' . $data->date . '&volume=' . $data->volume . '&issue=' . $data->issue . '&spage=' . $data->start_page . '&atitle=' . $data->title . '&stitle=' . $data->sub_title;

  $citation .= '<div class="scholar_coins_title"><span>  <br /><span class="Z3988" title="' . $coins_title . '"></span></div><div class="scholar_coins_url"><br/><A HREF="' . $coins_url . '"><img src="http://library.upei.ca/sites/all/files/sharedfolder/files/getit_upei.gif" /> </A></span><br /></div>';

  $tabset['first_tab']['tabs']['summary'] = array(
    '#type' => 'tabpage',
    '#title' => t('Citation'),
    '#content' => $citation,
    '#weight' => 1,
  );

  $tabset['first_tab']['tabs']['romeo'] = array(
    '#type' => 'tabpage',
    '#title' => t('Romeo'),
    '#content' => $romeo,
    '#weight' => 2,
  );

  if (user_access('add scholarly research') == TRUE && array_key_exists('PDF', $ds_array) == FALSE) {

    $tabset['first_tab']['tabs']['upload'] = array(
      '#type' => 'tabpage',
      '#title' => t('Upload'),
      '#content' => drupal_get_form('scholar_add_ir_datastream_form', $pid),
      '#weight' => 3,
    );
  }
  else {
    if (array_key_exists('PDF', $ds_array)) {
      $tabset['first_tab']['tabs']['pdf'] = array(
        '#type' => 'tabpage',
        '#title' => t('PDF'),
        '#content' => l(t('View PDF'), 'fedora/repository/' . $pid . '/PDF/PDF'),
        '#weight' => 3,
      );
    }
    else {
      $tabset['first_tab']['tabs']['upload'] = array(
        '#type' => 'tabpage',
        '#title' => t('PDF'),
        '#content' => t('No PDF has been uploaded yet'),
        '#weight' => 3,
      );
    }
  }

  if (user_access('administer scholarly research')) {

    if (array_key_exists('ZIP', $ds_array)) {

      $supplementary_content = '';

      $tabset['first_tab']['tabs']['supplementary'] = array(
        '#type' => 'tabpage',
        '#title' => t('Supplementary information'),
        '#content' => drupal_get_form('islandora_scholar_supp_info_delete_form', $pid),
        '#weight' => 6,
      );
    }
    else {
      $tabset['first_tab']['tabs']['supplementary'] = array(
        '#type' => 'tabpage',
        '#title' => t('Supplementary information'),
        '#content' => drupal_get_form('islandora_scholar_supp_info_form', $pid),
        '#weight' => 6,
      );
    }
  }
  return $tabset;
}

function islandora_scholar_zip_mods_to_array($mods) {

  $xpath = new DOMXPath($mods);
  $supp_note = $xpath->query("//*[@type='supplement']")->item(0)->nodeValue;
  if ($supp_note == NULL) {
    return FALSE;
  }
  $citation .= '<pre>' . str_replace(',', "</br>", $supp_note) . '</pre></br>';
  $supp_array = explode(',', $supp_note);
  $supp_no = count($supp_array);
  $filtered_array = array();
  $info_array = explode('  ', trim($supp_array[($supp_no - 1)]));
  $filtered_array['size'] = $info_array[0];
  $filtered_array['files'] = trim($info_array[(count($info_array) - 1)]);
  $line_array = array();
  $j = 0;
  for ($i = 3; $i < ($supp_no - 2); $i++) {
    $line_array[$j] = explode('  ', trim($supp_array[$i]));
    $filtered_array[$j]['name'] = $line_array[$j][2];
    $filtered_array[$j]['date'] = $line_array[$j][1];
    $filtered_array[$j]['size'] = $line_array[$j][0];
    $j++;
  }
  return $filtered_array;
}