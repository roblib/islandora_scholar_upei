<?php

/**
 * Provides the functions to process and view scholarly citations.
 * 
 * @file
 * @author Richard Wincewicz
 */
class Scholar {

  function __construct($pid = '') {
    module_load_include('inc', 'fedora_repository', 'api/fedora_item');
    if (!empty($pid)) {
      $this->pid = $pid;
      $this->item = new Fedora_Item($pid);
    }
  }

  /**
   * Build up the elements to show a basic citation. This currently
   * calls another function but customisation can be added in
   * 
   * @return array 
   */
  function showCitation() {
    $pid = $this->pid;
    $tabset = islandora_scholar_build_citation($pid, 'citation');
    return $tabset;
  }

  /**
   * Build up the elements to show a thesis. This uses the citation build function
   * as well as adding in some extra customisation such as the signature datastream
   * 
   * @return array
   */
  function showThesis() {
    $pid = $this->pid;
    $item = new Fedora_Item($pid);
    $datastream_array = $item->get_datastreams_list_as_array();
    $tabset = islandora_scholar_build_citation($pid, 'thesis');
    if (user_access('view signature tab')) {
      if (array_key_exists('SIGNATURE', $datastream_array)) {

        $tabset['first_tab']['tabs']['signature'] = array(
          '#type' => 'tabpage',
          '#title' => t('Signature'),
          '#content' => drupal_get_form('islandora_scholar_signature_delete_form', $pid),
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
    }

    return $tabset;
  }

  /**
   * This will build the view for a researcher object
   * @global type $base_url
   * @global type $base_path
   * @global type $user
   * @return type 
   */
  function show_researcher() {
    global $base_url;
    global $base_path;
    global $user;
    module_load_include('inc', 'fedora_repository', 'api/fedora_item');
    module_load_include('inc', 'scholar_tracking', 'scholar_tracking');
    module_load_include('inc', 'scholar', 'Users');
    $path = drupal_get_path('module', 'scholar');
    drupal_add_css($path . '/css/scholar.css');

    $solr_location = variable_get('islandora_solr_search_block_url', 'localhost:8080/solr');

    $collectionHelper = new CollectionClass();
    $xmlstr = $collectionHelper->getStream($this->pid, "MADS");

    if ($xmlstr != NULL) {
      $xml = new SimpleXMLElement($xmlstr);

      $names = $xml->authority->name->namePart;
      $fullname = $names[0] . ' ' . $names[1];

      foreach ($xml->affiliation->organization as $dept) {
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
    }
    else {
      drupal_set_message(t('No MADS datastream found!'), 'warning');
    }
//    drupal_set_title(t('@fullname', array('@fullname' => $fullname)));

    $tabset['first_tab'] = array(
      '#type' => 'tabpage',
      '#title' => t('Description'),
    );

    $tabset['first_tab']['tabs'] = array(
      '#type' => 'tabset',
    );

    $tabset['first_tab']['tabs']['details'] = array(
      '#type' => 'tabpage',
      '#title' => t('Details'),
      '#weight' => 0,
    );

    $tabset['first_tab']['tabs']['details']['tn'] = array(
      '#value' => '<img src="' . $base_url . '/' . $path . '/images/gnome_stock_person.png"/>',
    );

    $header = array();

    $rows = array(
      array('Name', $fullname),
      array('User name', $username),
      array('Email', $email),
      array('Department(s)', $dept),
    );

    $rss_feed = l('<img src="' . $base_url . '/' . $path . '/images/feed-icon-28x28.png">', 'rss/' . $username, array('html' => TRUE));

    $tabset['first_tab']['tabs']['details']['table'] = array(
      '#value' => theme('table', $header, $rows) . $rss_feed,
    );

    $tabset['first_tab']['tabs']['citations'] = array(
      '#type' => 'tabpage',
      '#title' => t('Citations'),
      '#weight' => 1,
    );

    $pids = array();
//    if ($citations_cache = cache_get('islandora_scholar_citation_cache')) {
//      $rows = unserialize($citations_cache->data);
//    }
//    else {
    if ($username) {
      $username_query = file_get_contents('http://' . $solr_location . '/select/?q=mods.username:' . (str_replace(';', ' ', (strtolower($username)))) . '&fl=PID&rows=10000');
      if ($xml != NULL) {
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
          foreach ($array as $arr) {
            $pid = (string) $arr->str;
            $pids[] = $pid;
          }
        }
        else {
          $rows = NULL;
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

    $tabset['first_tab']['tabs']['citations']['table'] = array(
      '#value' => drupal_get_form('scholar_author_bibliography_form', $username, $username, 'author-citations', $this->pid),
    );

    $tabset['first_tab']['tabs']['theses'] = array(
      '#type' => 'tabpage',
      '#title' => t('Theses'),
      '#weight' => 2,
    );

    $tabset['first_tab']['tabs']['theses']['table'] = array(
      '#value' => drupal_get_form('scholar_author_bibliography_form', $username, $username, 'author-theses', $this->pid),
    );

    $tabset['first_tab']['tabs']['metrics'] = array(
      '#type' => 'tabpage',
      '#title' => t('Metrics'),
      '#weight' => 3,
    );

    $tabset['first_tab']['tabs']['metrics']['content'] = array(
      '#value' => scholar_object_metrics_form($username, 'author'),
      '#prefix' => '<div class="scholar_metrics_table">',
      '#suffix' => '</div>',
    );

    return $tabset;
  }

  /**
   * This will build the view for a department object
   * @global type $base_url
   * @global type $base_path
   * @global type $user
   * @return type 
   */
  function show_department() {
    global $base_url;
    global $base_path;
    global $user;
    module_load_include('inc', 'fedora_repository', 'api/fedora_item');
    module_load_include('inc', 'scholar_tracking', 'scholar_tracking');
    module_load_include('inc', 'scholar', 'Users');
    $path = drupal_get_path('module', 'scholar');
    drupal_add_css($path . '/css/scholar.css');

    $tabset['first_tab'] = array(
      '#type' => 'tabpage',
      '#title' => t('Description'),
    );

    $tabset['first_tab']['tabs'] = array(
      '#type' => 'tabset',
    );

    $tabset['first_tab']['tabs']['details'] = array(
      '#type' => 'tabpage',
      '#title' => t('Details'),
      '#weight' => 0,
    );

    $tabset['first_tab']['tabs']['details']['tn'] = array(
      '#value' => '<img src="' . $base_url . '/' . $path . '/images/gnome_stock_person.png"/>',
    );

    $collectionHelper = new CollectionClass();
    $xmlstr = $collectionHelper->getStream($this->pid, "MADS");
    if ($xmlstr != NULL) {
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

      $tabset['first_tab']['tabs']['details']['table'] = array(
        '#value' => theme('table', $header, $rows),
      );
    }
    else {
      $tabset['first_tab']['tabs']['details']['table'] = array(
        '#value' => t('No MADS datastream found!'),
      );
    }

    $tabset['first_tab']['tabs']['citations'] = array(
      '#type' => 'tabpage',
      '#title' => t('Citations'),
      '#weight' => 1,
    );

    $tabset['first_tab']['tabs']['citations']['table'] = array(
      '#value' => drupal_get_form('scholar_author_bibliography_form', $names, $username, 'department-citations', $this->pid),
    );

    $tabset['first_tab']['tabs']['theses'] = array(
      '#type' => 'tabpage',
      '#title' => t('Theses'),
      '#weight' => 2,
    );

    $tabset['first_tab']['tabs']['theses']['table'] = array(
      '#value' => drupal_get_form('scholar_author_bibliography_form', $names, $username, 'department-theses', $this->pid),
    );

    $tabset['first_tab']['tabs']['metrics'] = array(
      '#type' => 'tabpage',
      '#title' => t('Metrics'),
      '#weight' => 3,
    );

    $tabset['first_tab']['tabs']['metrics']['content'] = array(
      '#value' => scholar_object_metrics_form($names, 'department'),
      '#prefix' => '<div class="scholar_metrics_table">',
      '#suffix' => '</div>',
    );

    return $tabset;
  }

}

/**
 * Construct the form to upload a new document and attach rights 
 * and usage information
 * @param type $form_state
 * @param type $pid
 * @return type 
 */
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

/**
 * Build the form to upload supplemental information and associate it with
 * the object
 * @param type $form_state
 * @param type $pid
 * @return string 
 */
function islandora_scholar_supp_info_form(&$form_state, $pid) {
  $form = array();

  $form['#attributes'] = array('enctype' => "multipart/form-data");

  $form['upload'] = array(
    '#type' => 'file',
    '#title' => t('Attach supplemental information'),
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

/**
 * Validate the upload form
 * @param type $form
 * @param type $form_state
 * @return type 
 */
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

/**
 * Submit the upload form and push the file into the ZIP datastream
 * @param type $form
 * @param type $form_state 
 */
function islandora_scholar_supp_info_form_submit($form, &$form_state) {
  module_load_include('inc', 'fedora_repository', 'api/fedora_item');
  $pid = $form_state['values']['pid'];
  $file = $form_state['values']['file'];
  $name = str_replace(' ', '_', $file->filename);
  $object = new Fedora_Item($pid);
  $object->add_datastream_from_file($file->filepath, 'ZIP', $name);
  $list = islandora_scholar_zip_list($file->filepath);
  $collectionHelper = new CollectionClass();
  $xmlstr = $collectionHelper->getStream($pid, "MODS", 0);

  $mods_xml = new DOMDocument();
  if ($mods_xml != NULL) {
    $mods_xml->loadXML($xmlstr);
    $xpath = new DOMXPath($mods_xml);
    $test = $xpath->query("//*[@type='supplement']")->item(0)->nodeValue;
    if ($test !== '') {
      $note = $mods_xml->createElement('note', implode(', ', $list));
      $note->setAttribute('type', 'supplement');
      $mods_element = $mods_xml->getElementsByTagName('mods')->item(0);
      $mods_element->appendChild($note);
      $object->modify_datastream_by_value($mods_xml->saveXML(), 'MODS', 'MODS Record', 'text/xml');
    }
    else {
      drupal_set_message(t('Entry for supplemental information already exists!'), 'warning');
    }
  }
  else {
    drupal_set_message(t('Could not find MODS datastream!'), 'warning');
  }
}

/**
 * Build form to delete the supplemental information so that
 * the MODS record is updated at the same time
 * @param type $form_state
 * @param type $pid
 * @return string 
 */
function islandora_scholar_supp_info_delete_form(&$form_state, $pid) {
  module_load_include('inc', 'fedora_repository', 'fedora_item');

  $form = array();

  $object = new Fedora_Item($pid);

  $datastream_array = $object->get_datastreams_list_as_array();

  if (array_key_exists('POLICY', $datastream_array)) {

    $policy_details = $object->get_datastream_info('POLICY');

    if ($policy_details->datastream->label == 'Embargo policy - Doc') {
      $form['supp_embargo'] = array(
        '#id' => 'supp_embargo',
        '#type' => 'submit',
        '#prefix' => '<p>',
        '#suffix' => '</p>',
        '#value' => t('Embargo supplemental information'),
      );
    }
    else {
      $form['supp_embargo_del'] = array(
        '#id' => 'supp_embargo_del',
        '#type' => 'submit',
        '#prefix' => '<p>',
        '#suffix' => '</p>',
        '#value' => t('Remove supplemental information embargo'),
      );
    }
  }
  else {
    $form['supp_embargo'] = array(
      '#id' => 'supp_embargo',
      '#type' => 'submit',
      '#prefix' => '<p>',
      '#suffix' => '</p>',
      '#value' => t('Embargo supplemental information'),
    );
  }

  $form['supp_delete'] = array(
    '#id' => 'supp_delete',
    '#type' => 'submit',
    '#prefix' => '<p>',
    '#suffix' => '</p>',
    '#value' => t('Delete supplemental information'),
  );

  $form['pid'] = array(
    '#type' => 'value',
    '#value' => $pid,
  );

  return $form;
}

/**
 * Submit the supplementary information delete form
 * @param type $form
 * @param type $form_state 
 */
function islandora_scholar_supp_info_delete_form_submit($form, &$form_state) {
  module_load_include('inc', 'fedora_repository', 'api/fedora_item');
  $pid = $form_state['values']['pid'];
  $object = new Fedora_Item($pid);
  $datastream_array = $object->get_datastreams_list_as_array();
  $path = drupal_get_path('module', 'scholar');

  switch ($form_state['clicked_button']['#id']) {
    case 'supp_delete':
      $object->purge_datastream('ZIP');
      $collectionHelper = new CollectionClass();
      $xmlstr = $collectionHelper->getStream($pid, "MODS", 0);

      $mods_xml = new DOMDocument();
      if ($mods_xml != NULL) {
        $mods_xml->loadXML($xmlstr);
        $list = $mods_xml->getElementsByTagName('note');
        $remove_nodes = array();
        foreach ($list as $element) {
          $attribute = $element->getAttribute('type');
          if ($attribute == 'supplement') {
            $remove_nodes[] = $element;
          }
        }
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
      else {
        drupal_set_message(t('No MODS datastream found!'), 'warning');
      }
      break;

    case 'supp_embargo':
      if (array_key_exists('POLICY', $datastream_array)) {
        $policy_details = $object->get_datastream_info('POLICY');
        switch ($policy_details->datastream->label) {
          case 'Embargo policy - Doc':
            $result1 = $object->purge_datastream('POLICY');
            $result = $object->add_datastream_from_file($path . '/xacml/both-embargo.xml', 'POLICY', 'Embargo policy - Both');
            if ($result1 != NULL) {
              drupal_set_message(t('Embargo policy for object \'@pid\' removed!', array('@pid' => $pid)), 'ok');
            }
            else {
              drupal_set_message(t('There was an error trying to remove the embargo policy for object \'@pid\'!', array('@pid' => $pid)), 'error');
            }
            break;
          case 'Embargo policy - Supp':
            drupal_set_message(t('Supplemental information embargo already exists!'), 'warning');
            break;
          case 'Embargo policy - Both':
          case 'POLICY':
            drupal_set_message(t('Supplemental information embargo already exists!'), 'warning');
            break;
          default:
            drupal_set_message(t('Unknown policy datastream, unable to add!'), 'warning');
        }
      }
      else {
        $result = $object->add_datastream_from_file($path . '/xacml/supplemental-embargo.xml', 'POLICY', 'Embargo policy - Supp');
      }
      if ($result != NULL) {
        drupal_set_message(t('Embargo policy for object \'@pid\' added!', array('@pid' => $pid)), 'ok');
      }
      else {
        drupal_set_message(t('There was an error trying to add the embargo policy for object \'@pid\'!', array('@pid' => $pid)), 'error');
      }
      break;
    case 'supp_embargo_del':
      $policy_details = $object->get_datastream_info('POLICY');
      switch ($policy_details->datastream->label) {
        case 'Embargo policy - Supp':
          $result = $object->purge_datastream('POLICY');
          break;
        case 'Embargo policy - Doc':
          drupal_set_message(t('Embargo is only set for the document, not removed!'), 'warning');
          $result = TRUE;
          break;
        case 'Embargo policy - Both':
        case 'POLICY':
          $result = $object->purge_datastream('POLICY');
          $result1 = TRUE;
          $result1 = $object->add_datastream_from_file($path . '/xacml/document-embargo.xml', 'POLICY', 'Embargo policy - Doc');
          if ($result1 != NULL) {
            drupal_set_message(t('Document embargo policy for object \'@pid\' added', array('@pid' => $pid)), 'ok');
          }
          else {
            drupal_set_message(t('An error occurred adding the new policy datastream!'), 'warning');
          }
          break;
        default:
          drupal_set_message(t('Unknown policy datastream, unable to remove!'), 'warning');
      }
      if ($result != NULL) {
        drupal_set_message(t('Embargo policy for object \'@pid\' removed!', array('@pid' => $pid)), 'ok');
      }
      else {
        drupal_set_message(t('There was an error trying to remove the embargo policy for object \'@pid\'!', array('@pid' => $pid)), 'error');
      }
      break;
  }
}

/**
 * Form to delete the associated documents and remove the relevant information
 * in the MODS record
 * @param type $form_state
 * @param type $pid
 * @return type 
 */
function islandora_scholar_document_delete_form(&$form_state, $pid) {

  module_load_include('inc', 'fedora_repository', 'api/fedora_item');
  module_load_include('inc', 'fedora_repository', 'ObjectHelper');
  module_load_include('inc', 'scholar', 'tracking/scholar_tracking');

  global $user;
  global $base_url;

  $form = array();

  $object = new Fedora_Item($pid);
  $datastream_array = $object->get_datastreams_list_as_array();
  if (module_exists('scholar_tracking')) {
    $track = new islandora_scholar_tracking($pid);
  }
  else {
    $track = FALSE;
  }
  $object_details = islandora_scholar_get_object_profile($pid);

  if (array_key_exists('OBJ', $datastream_array)) {
    if ($user->name === $object_details['owner'] || user_access('administer scholarly research')) {

      $obj_content = l(t('Download document'), 'download_ds/' . $pid . '/OBJ') . '</br>';

      if ($track != FALSE) {
        $obj_content .= theme('tracking', $pid, 'OBJ');
      }

      $form['obj_info'] = array(
        '#value' => $obj_content,
      );

      if (user_access('administer scholarly research')) {
        $form['obj_delete'] = array(
          '#type' => 'submit',
          '#name' => 'obj_delete',
          '#suffix' => '</br></br>',
          '#value' => t('Delete document'),
        );
      }
    }
  }

  if (array_key_exists('PDF', $datastream_array)) {

    $pdf_content = "<div><iframe src=\"http://docs.google.com/viewer?url=" . $base_url . '/fedora/repository/' . $pid . '/PDF/preview.pdf' . "&embedded=TRUE\" style=\"width:600px; height:500px;\" frameborder=\"0\"></iframe></div>";
    $pdf_content .= l(t('Download PDF'), 'download_ds/' . $pid . '/PDF') . '</br>';

    if ($track != FALSE) {
      $pdf_content .= theme('tracking', $pid, 'PDF');
    }

    $form['pdf_info'] = array(
      '#value' => $pdf_content,
    );

    if (user_access('administer scholarly research')) {
      $form['pdf_delete'] = array(
        '#type' => 'submit',
        '#name' => 'pdf_delete',
        '#suffix' => '</br></br>',
        '#value' => t('Delete PDF'),
      );
    }
  }
  else {
    $form['no_doc'] = array(
      '#type' => 'value',
      '#content' => t('No document has been uploaded yet'),
    );
  }

  if (array_key_exists('PDF', $datastream_array) || array_key_exists('OBJ', $datastream_array)) {
    if (user_access('administer scholarly research') || $user->name === $object_details['owner']) {
      if (!array_key_exists('POLICY', $datastream_array)) {
        $form['embargo'] = array(
          '#type' => 'submit',
          '#name' => 'embargo',
          '#value' => t('Embargo documents'),
        );
      }
      else {
        $policy_details = $object->get_datastream_info('POLICY');

        if ($policy_details->datastream->label != 'Embargo policy - Supp') {
          $form['remove_embargo'] = array(
            '#type' => 'submit',
            '#name' => 'remove_embargo',
            '#value' => t('Remove embargo'),
          );
        }
        else {
          $form['embargo'] = array(
            '#type' => 'submit',
            '#name' => 'embargo',
            '#value' => t('Embargo documents'),
          );
        }
      }
    }
  }

  $form['pid'] = array(
    '#type' => 'value',
    '#value' => $pid,
  );

  return $form;
}

/**
 * Submit the document delete form
 * @param type $form
 * @param type $form_state 
 */
function islandora_scholar_document_delete_form_submit($form, &$form_state) {
  module_load_include('inc', 'fedora_repository', 'api/fedora_item');

  $path = drupal_get_path('module', 'scholar');
  $pid = $form_state['values']['pid'];
  $object = new Fedora_Item($pid);
  $datastream_array = $object->get_datastreams_list_as_array();

  switch ($form_state['clicked_button']['#name']) {
    case 'obj_delete':
      $dsid = 'OBJ';
      break;
    case 'pdf_delete':
      $dsid = 'PDF';
      break;
    case 'embargo':
      $dsid = 'EMBARGO';
      break;
    case 'remove_embargo':
      $dsid = 'REMOVE_EMBARGO';
      break;
  }

  if ($dsid == 'OBJ' || $dsid == 'PDF') {
    $result = $object->purge_datastream($dsid);
    if ($result != NULL) {
      drupal_set_message(t("Datastream '@dsid' removed!", array('@dsid' => $dsid)));
    }
    else {
      drupal_set_message(t("An error occurred while trying to remove datastream '@dsid'", array('@dsid' => $dsid)), 'error');
    }
    $collectionHelper = new CollectionClass();
    $xmlstr = $collectionHelper->getStream($pid, "MODS", 0);

    if (($dsid == 'OBJ' && !array_key_exists('PDF', $datastream_array)) || ($dsid == 'PDF' && !array_key_exists('OBJ', $datastream_array))) {

      if ($xmlstr != NULL) {
        $mods_xml = new DOMDocument();
        $mods_xml->loadXML($xmlstr);
        $list = $mods_xml->getElementsByTagName('accessCondition');
        $remove_nodes = array();
        foreach ($list as $element) {
          $attribute = $element->getAttribute('type');
          if ($attribute == 'use and reproduction') {
            $remove_nodes[] = $element;
          }
        }
        $list2 = $mods_xml->getElementsByTagName('form');
        foreach ($list2 as $element2) {
          $attribute2 = $element2->getAttribute('authority');
          if ($attribute2 == 'local') {
            // This is crude as it removes everything in a 'physicalDescription
            // tag. If this becomes an issue this remove can be more specific.
            $remove_nodes[] = $element2;
          }
        }
        if (!empty($remove_nodes)) {
          foreach ($remove_nodes as $remove_node) {
            $remove_node->parentNode->removeChild($remove_node);
          }
          $object->modify_datastream_by_value($mods_xml->saveXML(), 'MODS', 'MODS Record', 'text/xml');
          drupal_set_message(t('MODS data relevant to this document has been removed'));
        }
        else {
          drupal_set_message(t('No MODS element found to remove!'));
        }
      }
      else {
        drupal_set_message(t('No MODS datastream found!'), 'warning');
      }
    }
  }
  else {
    if ($dsid == 'EMBARGO') {
      if (array_key_exists('POLICY', $datastream_array)) {
        $policy_details = $object->get_datastream_info('POLICY');
        drupal_set_message('Policy details: ' . $policy_details->datastream->label);
        switch ($policy_details->datastream->label) {
          case 'Embargo policy - Supp':
            $result1 = $object->purge_datastream('POLICY');
            $result = $object->add_datastream_from_file($path . '/xacml/both-embargo.xml', 'POLICY', 'Embargo policy - Both');
            if ($result1 != NULL) {
              drupal_set_message(t('Embargo policy for object \'@pid\' removed!', array('@pid' => $pid)), 'ok');
            }
            else {
              drupal_set_message(t('There was an error trying to remove the embargo policy for object \'@pid\'!', array('@pid' => $pid)), 'error');
            }
            break;
          case 'Embargo policy - Doc':
            drupal_set_message(t('Document embargo already exists!'), 'warning');
            break;
          case 'Embargo policy - Both':
          case 'POLICY':
            drupal_set_message(t('Document embargo already exists!'), 'warning');
            break;
          default:
            drupal_set_message(t('Unknown policy datastream, unable to add!'), 'warning');
        }
      }
      else {
        $result = $object->add_datastream_from_file($path . '/xacml/document-embargo.xml', 'POLICY', 'Embargo policy - Doc');
      }
      if ($result != NULL) {
        drupal_set_message(t('Embargo policy for object \'@pid\' added!', array('@pid' => $pid)), 'ok');
      }
      else {
        drupal_set_message(t('There was an error trying to add the embargo policy for object \'@pid\'!', array('@pid' => $pid)), 'error');
      }
    }
    if ($dsid == 'REMOVE_EMBARGO') {
      $policy_details = $object->get_datastream_info('POLICY');
      switch ($policy_details->datastream->label) {
        case 'Embargo policy - Doc':
          $result = $object->purge_datastream('POLICY');
          break;
        case 'Embargo policy - Supp':
          drupal_set_message(t('Embargo is only set for the supplemental information, not removed!'), 'warning');
          $result = TRUE;
          break;
        case 'Embargo policy - Both':
        case 'POLICY':
          $result = $object->purge_datastream('POLICY');
          $result1 = TRUE;
          $result1 = $object->add_datastream_from_file($path . '/xacml/supplemental-embargo.xml', 'POLICY', 'Embargo policy - Supp');
          if ($result1 != NULL) {
            drupal_set_message(t('Supplemental information embargo policy for object \'@pid\' added', array('@pid' => $pid)), 'ok');
          }
          else {
            drupal_set_message(t('An error occurred adding the new policy datastream!'), 'warning');
          }
          break;
        default:
          drupal_set_message(t('Unknown policy datastream, unable to remove!'), 'warning');
      }
      if ($result != NULL) {
        drupal_set_message(t('Embargo policy for object \'@pid\' removed!', array('@pid' => $pid)), 'ok');
      }
      else {
        drupal_set_message(t('There was an error trying to remove the embargo policy for object \'@pid\'!', array('@pid' => $pid)), 'error');
      }
    }
  }
}

/**
 * Get a list of files from a zip file
 * @param type $zip_file
 * @return type 
 */
function islandora_scholar_zip_list($zip_file) {
  exec('unzip -l "' . $zip_file . '"', $info);
  return $info;
}

/**
 * Form to upload a signature file
 * @param type $form_state
 * @param type $pid
 * @return type 
 */
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

/**
 * Validate the signature upload
 * @param type $form
 * @param type $form_state
 * @return type 
 */
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

/**
 * Submit the signature upload form
 * @param type $form
 * @param type $form_state 
 */
function islandora_scholar_signature_form_submit($form, &$form_state) {
  module_load_include('inc', 'fedora_repository', 'api/fedora_item');
  $pid = $form_state['values']['pid'];
  $file = $form_state['values']['file'];
  $object = new Fedora_Item($pid);
  drupal_set_message(t("File '@file' added", array('@file' => $file->filepath)));
  $object->add_datastream_from_file($file->filepath, 'SIGNATURE', 'SIGNATURE');
}

function islandora_scholar_signature_delete_form(&$form_state, $pid) {
  module_load_include('inc', 'scholar_tracking', 'scholar_tracking');

  $content = l(t('Signature file'), 'download_ds/' . $pid . '/SIGNATURE');

  if (module_exists('scholar_tracking')) {
    $content .= theme('tracking', $pid, 'SIGNATURE');
  }

  $form['content'] = array(
    '#value' => $content,
  );

  if (user_access('administer scholarly research')) {
    $form['sig_delete'] = array(
      '#type' => 'submit',
      '#name' => 'sig_delete',
      '#prefix' => '</br>',
      '#suffix' => '</br></br>',
      '#value' => t('Delete signature'),
    );
  }

  $form['pid'] = array(
    '#type' => 'value',
    '#value' => $pid,
  );

  return $form;
}

function islandora_scholar_signature_delete_form_submit($form, &$form_state) {
  module_load_include('inc', 'fedora_repository', 'api/fedora_item');
  $pid = $form_state['values']['pid'];
  $dsid = 'SIGNATURE';
  $object = new Fedora_Item($pid);
  $result = $object->purge_datastream($dsid);
  if ($result != NULL) {
    drupal_set_message(t("Datastream '@dsid' removed!", array('@dsid' => $dsid)));
  }
  else {
    drupal_set_message(t("An error occurred while trying to remove datastream '@dsid'", array('@dsid' => $dsid)), 'error');
  }
}

/**
 * Function to build the content required for viewing a citation
 * @todo Should split this out into smaller functions
 * @global type $base_url
 * @global type $base_path
 * @global type $user
 * @param type $pid PID of the object that contains the citation data
 * @param type $type The type of object, currently limited to citation or thesis
 * @return int 
 */
function islandora_scholar_build_citation($pid, $type) {

  global $base_url;
  global $base_path;
  global $user;
  module_load_include('inc', 'fedora_repository', 'ObjectHelper');
  module_load_include('inc', 'fedora_repository', 'api/fedora_item');
  module_load_include('inc', 'citeproc', 'CiteProcJSBibliography');
  module_load_include('inc', 'scholar', '/citation/bibliography/BibliographyForm');
  module_load_include('php', 'scholar', 'IrClass');
  module_load_include('inc', 'scholar', '/citation/bibliography/CitationForm');
  module_load_include('inc', 'Fedora_Repository', 'CollectionClass');
  module_load_include('inc', 'scholar', 'Coins');
  module_load_include('inc', 'islandora_bibliography', 'Bibliography');
  module_load_include('inc', 'scholar', 'Users');
  drupal_add_css(drupal_get_path('module', 'islandora_bibliography') . '/css/export.css');
  drupal_add_css(drupal_get_path('module', 'scholar') . '/css/scholar.css');

  $data = new Coins($pid, $type);
  $item = new Fedora_Item($pid);
  $datastream_array = $item->get_datastreams_list_as_array();

  drupal_set_html_head('<meta name="citation_title" content="' . $data->title . '" />');
  foreach ($data->reverse_names as $name) {
    drupal_set_html_head('<meta name="citation_author" content="' . $name . '" />');
  }
  drupal_set_html_head('<meta name="citation_publication_date" content="' . $data->date . '" />');
  drupal_set_html_head('<meta name="citation_journal_title" content="' . $data->journal_title . '" />');
  drupal_set_html_head('<meta name="citation_volume" content="' . $data->volume . '" />');
  drupal_set_html_head('<meta name="citation_issue" content="' . $data->issue . '" />');
  drupal_set_html_head('<meta name="citation_firstpage" content="' . $data->start_page . '" />');
  if (array_key_exists('PDF', $datastream_array)) {
    drupal_set_html_head('<meta name="citation_pdf_url" content="' . $base_url . '/download_ds/' . $pid . '/PDF" />');
  }

  $fedora_url = variable_get('fedora_base_url', 'http://localhost:8080/fedora');

  $object_details = islandora_scholar_get_object_profile($pid);

  $path = drupal_get_path('module', 'Fedora_Repository');

  $tabset['first_tab'] = array(
    '#type' => 'tabpage',
    '#title' => t('Description'),
  );

  $tabset['first_tab']['tabs'] = array(
    '#type' => 'tabset',
  );

  $tabset['first_tab']['tabs']['summary'] = array(
    '#type' => 'tabpage',
    '#title' => t('Citation'),
    '#content' => theme('scholar', $pid, $type),
    '#weight' => 1,
  );

  $tabset['first_tab']['tabs']['summary']['citation_button'] = array(
    '#prefix' => '<div class="scholar_bibliography_add_form">',
    '#suffix' => '</div>',
    '#value' => drupal_get_form('islandora_bibliography_citation_form', $pid),
  );

  if (user_access('add scholarly research') == TRUE) {
    $tabset['first_tab']['tabs']['summary']['associate_button'] = array(
      '#prefix' => '<div class="scholar_associate_button">',
      '#suffix' => '</div>',
      '#value' => drupal_get_form('islandora_scholar_associate_button_form', $pid),
    );
  }

  if (array_key_exists('ZIP', $datastream_array)) {
    $datastream_info = $item->get_datastream_info('ZIP');
    $description = $datastream_array['ZIP']['label'];
    $mods = islandora_bibilgraphy_form_get_mods($pid);
    $supp_array = islandora_scholar_zip_mods_to_array($mods);

    if ($supp_array) {

      $tabset['first_tab']['tabs']['summary']['supplemental_info'] = array(
        '#type' => 'fieldset',
        '#prefix' => '<div class="scholar_supplemental_table">',
        '#suffix' => '</div>',
        '#title' => t('Details'),
        '#weight' => -1,
        '#collapsible' => TRUE,
        '#collapsed' => TRUE,
      );

      $tabset['first_tab']['tabs']['summary']['supplemental_info']['info'] = array(
        '#prefix' => '<div>',
        '#suffix' => '</div>',
        '#value' => theme('supplemental', $pid),
      );
    }
    else {
      drupal_set_message(t('A supplemental information file is present but there is no record of its details, maybe something went wrong during upload'), 'warning');
    }
  }

  if (user_is_logged_in() && $data->issn != NULL && variable_get('scholar_romeo_check', 1) == 1) {

    $tabset['first_tab']['tabs']['romeo'] = array(
      '#type' => 'tabpage',
      '#title' => t('RoMEO'),
      '#content' => scholar_get_romeo($data->issn),
      '#weight' => 2,
    );
  }

  if (user_access('add scholarly research') == TRUE) {
    if (array_key_exists('OBJ', $datastream_array) == FALSE && array_key_exists('PDF', $datastream_array) == FALSE) {

      $tabset['first_tab']['tabs']['upload'] = array(
        '#type' => 'tabpage',
        '#title' => t('Upload'),
        '#content' => drupal_get_form('scholar_add_ir_datastream_form', $pid),
        '#weight' => 3,
      );
    }
    else {
      $tabset['first_tab']['tabs']['pdf'] = array(
        '#type' => 'tabpage',
        '#title' => t('Document'),
        '#content' => drupal_get_form('islandora_scholar_document_delete_form', $pid),
        '#weight' => 3,
      );
    }
  }
  /** else {

    if (array_key_exists('OBJ', $datastream_array) == FALSE && array_key_exists('PDF', $datastream_array) == FALSE) {
    $tabset['first_tab']['tabs']['pdf'] = array(
    '#type' => 'tabpage',
    '#title' => t('Document'),
    '#content' => t('No documents have been uploaded yet.'),
    '#weight' => 3,
    );
    }
    else {
    $tabset['first_tab']['tabs']['pdf'] = array(
    '#type' => 'tabpage',
    '#title' => t('Document'),
    '#content' => drupal_get_form('islandora_scholar_document_delete_form', $pid),
    '#weight' => 3,
    );
    }
    } */
  if (user_access('administer scholarly research')) {

    if (array_key_exists('ZIP', $datastream_array)) {

      $tabset['first_tab']['tabs']['supplementary'] = array(
        '#type' => 'tabpage',
        '#title' => t('Supplemental information'),
        '#content' => drupal_get_form('islandora_scholar_supp_info_delete_form', $pid),
        '#weight' => 6,
      );

      if (module_exists('scholar_tracking')) {
        $tabset['first_tab']['tabs']['supplementary']['tracking'] = array(
          '#value' => theme('tracking', $pid, 'ZIP'),
        );
      }
    }
    else {
      $tabset['first_tab']['tabs']['supplementary'] = array(
        '#type' => 'tabpage',
        '#title' => t('Supplemental information'),
        '#content' => drupal_get_form('islandora_scholar_supp_info_form', $pid),
        '#weight' => 6,
      );
    }
  }
//  var_dump($_SESSION);
  return $tabset;
}

/**
 * Function to process the information about the supplemental information contained 
 * within the ZIP file so it can be added to the citation view
 * 
 * @param type $mods
 * MODS record from the Fedora object
 * 
 * @return type
 * Array of values that can be easily displayed in a table
 */
function islandora_scholar_zip_mods_to_array($mods) {

  $xpath = new DOMXPath($mods);
  $supplemental_note = $xpath->query("//*[@type='supplement']")->item(0)->nodeValue;
  if ($supplemental_note == NULL) {
    return FALSE;
  }
  $supplemental_array = explode(',', $supplemental_note);
  $supplemental_no = count($supplemental_array);
  $info_array = explode('  ', trim($supplemental_array[($supplemental_no - 1)]));
  $filtered_array = array(
    'size' => $info_array[0],
    'files' => trim($info_array[(count($info_array) - 1)])
  );
  $line_array = array();
  $j = 0;
  for ($i = 3; $i < ($supplemental_no - 2); $i++) {
    $line_array[$j] = explode('  ', trim($supplemental_array[$i]));
    $filtered_array[$j] = array(
      'name' => $line_array[$j][2],
      'date' => $line_array[$j][1],
      'size' => $line_array[$j][0],
    );
    $j++;
  }
  return $filtered_array;
}

/**
 * Function that returns an array with details about a specific object
 * This isn't possible with the current Islandora API
 * 
 * @todo Work this into the Fedora API
 * 
 * @param string $pid
 * PID of the object
 * 
 * @return array
 * Array containing details about the Fedora object
 */
function islandora_scholar_get_object_profile($pid) {
  $fedora_url = variable_get('fedora_base_url', 'http://localhost:8080/fedora');
  $url = $fedora_url . '/objects/' . $pid . '?format=xml';
  $result = file_get_contents($url);
  if ($result) {
    $result_xml = new SimpleXMLElement($result);
    $details = array(
      'owner' => $result_xml->objOwnerId,
      'label' => $result_xml->objLabel,
      'created' => $result_xml->objCreateDate,
      'modified' => $result_xml->objLastModDate,
      'status' => $result_xml->objState,
    );
    return $details;
  }
  else {
    drupal_set_message(t('Could not find details for object @pid!', array('@pid' => $pid)), 'warning');
    return NULL;
  }
}

function tstamptotime($tstamp) {
  // converts ISODATE to unix date
  // 1984-09-01T14:21:31Z
  sscanf($tstamp, "%u-%u-%uT%u:%u:%uZ", $year, $month, $day, $hour, $min, $sec);
  $newtstamp = mktime($hour, $min, $sec, $month, $day, $year);
  return $newtstamp;
}

/**
 * Provides the HTML for displaying RoMEO information about a citation.
 * 
 * @param string $issn
 *   ISSN of the citation. Usually pulled from the Refworks or MODS record
 * 
 * @return string 
 *   HTML which can be displayed in a drupal form
 */
function scholar_get_romeo($issn) {

  $path = drupal_get_path('module', 'Fedora_Repository');
  $base_url = variable_get('scholar_romeo_url', 'http://www.sherpa.ac.uk/romeo/api29.php?issn=');
  $api_key = '&ak=' . variable_get('scholar_romeo_key', '');

  if ($romeo_cache = cache_get('islandora_scholar_romeo_cache_' . $issn)) {
    $romeo = unserialize($romeo_cache->data);
  }
  else {
    $romeo_url = file_get_contents($base_url . $issn . $api_key);
    try {
      $processor = new XsltProcessor();
    } catch (Exception $e) {
      drupal_set_message(t('@error', array('@error' => $e->getMessage())), 'error');
      return;
    }
    $xsl = new DOMDocument();
    $xsl->load($path . '/xsl/romeo.xsl');
    $input = new DOMDocument();
    if ($romeo_url) {
      $input->loadXML(trim($romeo_url));
      $xsl = $processor->importStylesheet($xsl);
      $xml_output = $processor->transformToDoc($input);
      $romeo = $xml_output->saveXML();
      $cache_time = variable_get('scholar_romeo_cache_time', '2');
      cache_set('islandora_scholar_romeo_cache_' . $issn, serialize($romeo), 'cache', time() + ($cache_time * 86400));
    }
    else {
      drupal_set_message(t('Could not retrieve RoMEO information for ISSN: @issn!', array('@issn' => $issn)), 'warning');
      return NULL;
    }
  }
  return $romeo;
}

/**
 * Preprocess function for displaying the citations
 * 
 * @param array $variables 
 */
function template_preprocess_scholar(&$variables) {
  module_load_include('inc', 'fedora_repository', 'api/fedora_item');
  module_load_include('inc', 'scholar', 'Coins');
  module_load_include('inc', 'citeproc', 'CiteProcJSBibliography');
  module_load_include('inc', 'scholar', '/citation/bibliography/BibliographyForm');
  module_load_include('inc', 'scholar_tracking', 'scholar_tracking');

  global $base_url;

  $data = new Coins($variables['pid']);
  $table_values = $data->render_view($variables['type']);

  $csl = variable_get('scholar_default_csl', 'default');
  $style = citeproc_style($csl);
  $bibliography = new CiteProcJSBibliography($style);
  $mods = islandora_bibilgraphy_form_get_mods($variables['pid']);
  if ($mods) {
    $ref = citeproc_citation_from_mods($mods);
    $bibliography->addCitation($ref);
  }
  $citation = $bibliography->render();

  if (module_exists('scholar_tracking')) {

    $track = new islandora_scholar_tracking($variables['pid']);
    $track->add_page_count();
    $read = $track->read_page_count();
    $time_last_viewed = $read[($read['count'] - 1)];
    if ($time_last_viewed == NULL) {
      $time_last_viewed = 'Never';
    }
  }
  $path = $path = drupal_get_path('module', 'scholar');
  $item = new Fedora_Item($variables['pid']);
  $datastream_array = $item->get_datastreams_list_as_array();
  if (array_key_exists('OBJ', $datastream_array) && $data->status == 'FULL_TEXT_AVAILABLE') {//check status in case ds exists but is embargoed
    $check = 'OBJ';
  }
  if (array_key_exists('PDF', $datastream_array) && $data->status == 'FULL_TEXT_AVAILABLE') {
    $check = 'PDF';
  }
  $url_frag = 'fedora/repository/';
  if (module_exists('scholar_tracking')) {
    $url_frag = 'download_ds/';
  }
  switch ($check) {
    case 'OBJ':
      $download_img = $base_url . '/' . $path . '/images/download.png';
      $pdf_result = '<div class="scholar_pdf">' . l('<img src="' . $download_img . '"/> Full text', $url_frag . $variables['pid'] . '/OBJ/OBJ', array('html' => TRUE)) . '</div>';
      break;
    case 'PDF':
      $download_img = $base_url . '/' . $path . '/images/download.png';
      $pdf_result = '<div class="scholar_pdf">' . l('<img src="' . $download_img . '"/> Full text', $url_frag . $variables['pid'] . '/PDF/PDF', array('html' => TRUE)) . '</div>';
      break;
  }

  if (array_key_exists('ZIP', $datastream_array)) {
    $datastream_info = $item->get_datastream_info('ZIP');
    $description = $datastream_array['ZIP']['label'];

    $supp_array = islandora_scholar_zip_mods_to_array($mods);
    if ($supp_array) {
      $supp_header = array('Filename', 'Date', 'Size (bytes)');
      $supp_rows = array();
      for ($i = 0; $i < count($supp_array); $i++) {
        $supp_rows[] = $supp_array[$i];
      }
    }
    else {
      drupal_set_message(t('A supplemental information file is present but there is no record of its details, maybe something went wrong during upload'), 'warning');
    }
  }

  $variables = array(
    'pid' => $variables['pid'],
    'headers' => $table_values['headers'],
    'rows' => $table_values['rows'],
    'citation' => $citation,
    'server_ip' => $_SERVER['SERVER_ADDR'],
    'base_url' => $base_url,
    'date' => $data->date,
    'issn' => $data->issn,
    'volume' => $data->volume,
    'issue' => $data->issue,
    'start_page' => $data->start_page,
    'title' => $data->title,
    'journal_title' => $data->journal_title,
    'times_read' => $read['count'],
    'time_last_viewed' => $time_last_viewed,
    'datastream_array' => $datastream_array,
    'csl' => $csl,
    'full_text' => $pdf_result,
  );
}

/**
 * Preprocess function for displaying information about the stored supplemental info
 * 
 * @param type $variables 
 */
function template_preprocess_supplemental(&$variables) {
  module_load_include('inc', 'fedora_repository', 'api/fedora_item');

  $item = new Fedora_Item($variables['pid']);
  $datastream_array = $item->get_datastreams_list_as_array();

  if (array_key_exists('ZIP', $datastream_array)) {
    $datastream_info = $item->get_datastream_info('ZIP');
    $description = $datastream_array['ZIP']['label'];
    $mods = islandora_bibilgraphy_form_get_mods($variables['pid']);

    $supp_array = islandora_scholar_zip_mods_to_array($mods);
    if ($supp_array) {
      $supp_header = array('Filename', 'Date', 'Size (bytes)');
      $supp_rows = array();
      for ($i = 0; $i < count($supp_array); $i++) {
        if ($supp_array[$i]) {
          $supp_rows[] = $supp_array[$i];
        }
      }
    }
    else {
      drupal_set_message(t('A supplemental information file is present but there is no record of its details, maybe something went wrong during upload'), 'warning');
    }
  }

  $variables = array(
    'datastream_array' => $datastream_array,
    'zip_description' => $description,
    'zip_size' => $datastream_info->datastream->size,
    'zip_array' => $supp_array,
    'supp_header' => $supp_header,
    'supp_rows' => $supp_rows,
  );
}

/**
 * Preprocessor function for displaying the scholar tracking details
 * 
 * @param type $variables 
 */
function template_preprocess_tracking(&$variables) {
  module_load_include('inc', 'scholar_tracking', 'scholar_tracking');

  $track = new islandora_scholar_tracking($variables['pid']);
  $ds_count = $track->read_ds_count($variables['dsid']);
  $tracking_date = $ds_count[($ds_count['count'] - 1)];
  if (!$tracking_date) {
    $tracking_date = 'Never';
  }
  $variables = array(
    'tracking_count' => $ds_count['count'],
    'tracking_date' => $tracking_date,
  );
}