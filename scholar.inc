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
//   Edited by rwincewicz on 25 Oct 2011
//   ROMEO changed the URL they were using and so we had to update our code. Before the URL was stored as an external
//   datastream in Fedora and so would require every object to have the ROMEO datastream edited. This code now brings
//   the xml from the ROMEO API using a URL that is created dynamically from the code below. Redmine #5615
//   URL and API key for the ROMEO REST client. TODO: Add this to the admin panel so that users can change it easily.

    global $base_url;
    global $base_path;
    global $user;
    module_load_include('inc', 'fedora_repository', 'ObjectHelper');
    module_load_include('inc', 'fedora_repository', 'api/fedora_item');
    module_load_include('inc', 'citeproc', 'CiteProcJSBibliography');
    module_load_include('inc', 'scholar', '/citation/bibliography/BibliographyForm');
    module_load_include('php', 'scholar', 'IrClass');
    module_load_include('inc', 'scholar', '/citation/bibliography/CitationForm');
    drupal_add_css(drupal_get_path('module', 'islandora_bibliography') . '/css/export.css');

    $fedora_url = variable_get('fedora_base_url', 'http://localhost:8080/fedora');

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
    module_load_include('inc', 'Fedora_Repository', 'CollectionClass');
    module_load_include('inc', 'scholar', 'Coins');
    $data = new Coins($this->pid);
//    Get MODS datastream and pull the ISSN from there.

    $csl = 'APA';

    $style = citeproc_style($csl);
    $bibliography = new CiteProcJSBibliography($style);
    $mods = islandora_bibilgraphy_form_get_mods($this->pid);
    if ($mods) {
      $ref = citeproc_citation_from_mods($mods);
      $bibliography->addCitation($ref);
    }
    $citation = $bibliography->render();

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

    $citation .= '<span>  <br /><span class="Z3988" title="' . $coins_title . '"></span> <br/><A HREF="' . $coins_url . '"><img src="http://library.upei.ca/sites/all/files/sharedfolder/files/getit_upei.gif" /> </A></span><br />';

    $citation .= '<p><b>Abstract</b></p>';

    $citation .= $data->abstract;

    module_load_include('inc', 'islandora_bibliography', 'Bibliography');

    $pid = $this->pid;

    $citation .= drupal_get_form('islandora_bibliography_citation_form', $pid);

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

    if (user_access('add scholarly research') == TRUE) {

      $tabset['first_tab']['tabs']['upload'] = array(
        '#type' => 'tabpage',
        '#title' => t('Upload'),
        '#content' => drupal_get_form('scholar_add_ir_datastream_form', $pid),
        '#weight' => 3,
      );
    }

    $item = new Fedora_Item($pid);
    $ds_array = $item->get_datastreams_list_as_array();
    if (array_key_exists('PDF', $ds_array)) {
      $tabset['first_tab']['tabs']['pdf'] = array(
        '#type' => 'tabpage',
        '#title' => t('PDF'),
        '#content' => 'PDF',
        '#weight' => 4,
      );
    }

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