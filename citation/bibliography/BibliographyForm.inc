<?php

/**
 * @file
 *
 * Functions for including the Manage Bibliography form.
 */

/**
 * This form is called from the menu callback for the 'My Bibliography' page.
 *
 * From here the user can add/remove and export citations.
 *
 * @param array $form_state
 *   The drupal form state.
 *
 * @return array
 *   The drupal form.
 */
function islandora_bibliography_form(array &$form_state) {
  module_load_include('inc', 'islandora_bibliography', 'Bibliography');
  citation_exporter_include_download_redirect();
  $pids = Bibliography::GetCitations();
  if (empty($pids)) {
    islandora_bibilgraphy_form_empty_page();
  }
  else {
    module_load_include('inc', 'csl', 'CSL');
    drupal_add_css(drupal_get_path('module', 'islandora_bibliography') . '/css/bibliography.css');
    $styles = CSL::GetNames();
    return array(
      'fieldset' => array(
        '#type' => 'fieldset',
        '#title' => t('Citations'),
        'step_1' => array(
          '#value' => t('Step 1: Select your citations'),
          '#prefix' => '<h3>',
          '#suffix' => '</h3>'
        ),
        'table' => islandora_bibilgraphy_form_table($pids),
        'remove' => array(
          '#type' => 'submit',
          '#value' => t('Remove Selected'),
          '#submit' => array('islandora_bibliography_form_remove')
        ),
        'step_2' => array(
          '#value' => t('Step 2: Export the formatted citations'),
          '#prefix' => '<h3>',
          '#suffix' => '</h3>'
        ),
        'format' => array(
          '#title' => t('Format'),
          '#type' => 'select',
          '#length' => 50,
          '#options' => array(
            'HTML' => 'HTML',
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
//          'preview' => array(
//          '#type' => 'submit',
//          '#prefix' => '</br>',
//          '#value' => t('Preview Selected'),
//          '#submit' => array('islandora_bibliography_form_preview')
//        ),
        'export' => array(
          '#type' => 'submit',
          '#action' => 'export',
          '#export' => 'selected',
          '#value' => t('Export Selected'),
          '#submit' => array('islandora_bibliography_form_export')
        )
      )
    );
  }
}

/**
 * The form to render when no citations are in the bibliography.
 * 
 * @return array
 *   The drupal form
 */
function islandora_bibilgraphy_form_empty_page() {
  drupal_set_message(t('You have no citations in your bibliography.'));
  return array(
    'redirect' => array(
      '#prefix' => '<p>',
      '#value' => l(t('Return to the Home Page.'), ''),
      '#suffix' => '</p>'
    )
  );
}

/**
 * Generates the table for the bibliography form.
 * 
 * @param array $pids
 *   A collection of citations within the bibliography.
 * 
 * @return array
 *   The drupal form table definition.
 */
function islandora_bibilgraphy_form_table(array $pids) {
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
  foreach ($pids as $pid) {
    list($title, $authors) = islandora_bibilgraphy_form_get_title_authors($pid);
    $rows[] = array(
      '#pid' => $pid,
      'title' => array('#value' => l($title, 'fedora/repository/' . $pid, array('html' => TRUE))),
      'author' => array('#value' => check_plain($authors)),
    );
  }
  return $table;
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
function islandora_bibilgraphy_form_get_title_authors($pid) {
  $title = $authors = 'Missing';
  $mods = islandora_bibilgraphy_form_get_mods($pid);
  if ($mods) {
    $mods = simplexml_import_dom($mods);
    $mods->registerXPathNamespace('mods', 'http://www.loc.gov/mods/v3');
    $title = implode(' ', $mods->xpath('/mods:mods/mods:titleInfo/mods:title'));
    $authors = implode(', ', $mods->xpath('/mods:mods/mods:name/mods:role[mods:roleTerm = "author"]/../mods:namePart[@type="family"]'));
    if ($authors == '') {
      $authors = implode(', ', $mods->xpath('/mods:mods/mods:name/mods:role[mods:roleTerm = "aut"]/../mods:namePart[@type="family"]'));
    }
  }
  return array($title, $authors);
}

/**
 * Removes one citation from the bibliography.
 *
 * @param array $form
 *   The drupal form.
 * @param array $form_state
 *   The drupal form state.
 */
function islandora_bibliography_form_remove_citation_submit(array $form, array &$form_state) {
  module_load_include('inc', 'islandora_bibliography', 'Bibliography');
  $selections = $form_state['values']['table']['selections'];
  foreach ($selections as $pid) {
    Bibliography::RemoveCitation($pid);
  }
  drupal_set_message(t('Removed Citation(s)'));
}

/**
 * Previews the selected citations from the bibliography.
 *
 * @param array $form
 *   The drupal form.
 * @param array $form_state
 *   The drupal form state.
 */
function islandora_bibliography_form_preview(array $form, array &$form_state) {
  module_load_include('inc', 'citeproc', 'CiteProcJSBibliography');
  drupal_add_css(drupal_get_path('module', 'islandora_bibliography') . '/css/export.css');
  $selections = $form_state['values']['table']['selections'];
  $style = citeproc_style($form_state['values']['style']);
  $bibliography = new CiteProcJSBibliography($style);
  $selections = array_filter($selections);
  $selections = array_keys($selections);
  
  $output = <<<HTML
 <!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en" lang="en" dir="ltr">

<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />

  <title>My List | Scholar test</title>
  <meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<head> 
<script type='text/javascript; e4x=1' src='/sites/default/modules/islandora_scholar_upei/citation/citeproc/lib/citeproc-js/xmle4x.js'></script>
<link rel="shortcut icon" href="/misc/favicon.ico" type="image/x-icon" />
  <link type="text/css" rel="stylesheet" media="all" href="/modules/aggregator/aggregator.css?3" />
<link type="text/css" rel="stylesheet" media="all" href="/modules/node/node.css?3" />
<link type="text/css" rel="stylesheet" media="all" href="/modules/system/defaults.css?3" />
<link type="text/css" rel="stylesheet" media="all" href="/modules/system/system.css?3" />
<link type="text/css" rel="stylesheet" media="all" href="/modules/system/system-menus.css?3" />
<link type="text/css" rel="stylesheet" media="all" href="/modules/user/user.css?3" />
<link type="text/css" rel="stylesheet" media="all" href="/sites/all/modules/islandora_solr_search/css/islandora_solr_search.css?3" />
<link type="text/css" rel="stylesheet" media="all" href="/sites/default/modules/islandora_scholar_upei/citation/bibliography/css/bibliography.css?3" />
<link type="text/css" rel="stylesheet" media="all" href="/sites/default/modules/islandora_scholar_upei/citation/bibliography/css/export.css?3" />
<link type="text/css" rel="stylesheet" media="all" href="/sites/default/modules/islandora_scholar_upei/citation/citeproc/css/citeproc.css?3" />
<link type="text/css" rel="stylesheet" media="all" href="/sites/all/themes/vretheme/css/html-reset.css?3" />
<link type="text/css" rel="stylesheet" media="all" href="/sites/all/themes/vretheme/css/typography.css?3" />
<link type="text/css" rel="stylesheet" media="all" href="/sites/all/themes/vretheme/css/layout-fixed.css?3" />
<link type="text/css" rel="stylesheet" media="all" href="/sites/all/themes/vretheme/css/page-backgrounds.css?3" />
<link type="text/css" rel="stylesheet" media="all" href="/sites/all/themes/vretheme/css/tabs.css?3" />
<link type="text/css" rel="stylesheet" media="all" href="/sites/all/themes/vretheme/css/messages.css?3" />
<link type="text/css" rel="stylesheet" media="all" href="/sites/all/themes/vretheme/css/pages.css?3" />
<link type="text/css" rel="stylesheet" media="all" href="/sites/all/themes/vretheme/css/blocks.css?3" />
<link type="text/css" rel="stylesheet" media="all" href="/sites/all/themes/vretheme/css/navigation.css?3" />
<link type="text/css" rel="stylesheet" media="all" href="/sites/all/themes/vretheme/css/views-styles.css?3" />
<link type="text/css" rel="stylesheet" media="all" href="/sites/all/themes/vretheme/css/jcarousel.css?3" />
<link type="text/css" rel="stylesheet" media="all" href="/sites/all/themes/vretheme/css/nodes.css?3" />
<link type="text/css" rel="stylesheet" media="all" href="/sites/all/themes/vretheme/css/comments.css?3" />
<link type="text/css" rel="stylesheet" media="all" href="/sites/all/themes/vretheme/css/forms.css?3" />
<link type="text/css" rel="stylesheet" media="all" href="/sites/all/themes/vretheme/css/superfish.css?3" />
<link type="text/css" rel="stylesheet" media="all" href="/sites/all/themes/vretheme/css/islandora.css?3" />
<link type="text/css" rel="stylesheet" media="all" href="/sites/all/themes/vretheme/corporate/css/corporate.css?3" />
<link type="text/css" rel="stylesheet" media="all" href="/sites/all/themes/vretheme/corporate/colors/orange.css?3" />
<link type="text/css" rel="stylesheet" media="print" href="/sites/all/themes/vretheme/css/print.css?3" />
<!--[if lte IE 6]>
<link type="text/css" rel="stylesheet" media="all" href="/sites/all/themes/vretheme/css/ie6.css?3" />
<![endif]-->
<!--[if lte IE 7]>
<link type="text/css" rel="stylesheet" media="all" href="/sites/all/themes/vretheme/css/ie7.css?3" />
<![endif]-->
<!--[if IE]>
<link type="text/css" rel="stylesheet" media="all" href="/sites/all/themes/vretheme/css/ie.css?3" />
<![endif]-->
  <script type="text/javascript" src="/sites/all/modules/jquery_update/replace/jquery.min.js?3"></script>
<script type="text/javascript" src="/misc/drupal.js?3"></script>
<script type="text/javascript" src="/misc/tableselect.js?3"></script>
<script type="text/javascript" src="/sites/default/modules/islandora_scholar_upei/citation/citeproc/lib/citeproc-js/xmldom.js?3"></script>
<script type="text/javascript" src="/sites/default/modules/islandora_scholar_upei/citation/citeproc/lib/citeproc-js/citeproc.js?3"></script>
<script type="text/javascript" src="/sites/default/modules/islandora_scholar_upei/citation/citeproc/js/loadsys.js?3"></script>
<script type="text/javascript" src="/sites/default/modules/islandora_scholar_upei/citation/citeproc/js/runcites.js?3"></script>
<script type="text/javascript" src="/sites/all/themes/vretheme/js/jquery_example/jquery.example.min.js?3"></script>
<script type="text/javascript" src="/sites/all/themes/vretheme/js/searchtext.js?3"></script>
<script type="text/javascript" src="/sites/all/themes/vretheme/js/superfish.js?3"></script>
<script type="text/javascript" src="/sites/all/themes/vretheme/js/hoverIntent.js?3"></script>
HTML;
 
//    foreach ($selections as $pid) {
//    $mods = islandora_bibilgraphy_form_get_mods($pid);
//    if ($mods) {
//      $citation = citeproc_citation_from_mods($mods);
//      $bibliography->addCitation($citation);
//    }
//  }
  
//      $item_id = $bibliography->getID();
  
  $output .= '
<script type="text/javascript">
<!--//--><![CDATA[//><!--
jQuery.extend(Drupal.settings, ' . convert_mods_to_citeproc_json($mods, $item_id) . ' ); 
//--><!]]>
</script>
';
  
  $output .= <<<HTML2
<script type="text/javascript">
  jQuery(document).ready(function($) {
    $("#superfish ul.menu").superfish({ 
            delay:       100,                           
            animation:   {opacity:'show'},  
            speed:       'fast',                          
            autoArrows:  true,                           
            dropShadows: true                   
        });
  });
</script>
</head>
<body>
HTML2;
 
  if (isset($selections[0])) {
    $output .= $bibliography->render();
  }
  else {
    drupal_set_message(t('No citations were selected!'), 'warning');
  }
  $output .= l(t('Return to the Bibliography.'), 'bibliography');
  $output .= '</body></html>';
  print $output;
  exit();
}

/**
 * Removes the selected citations from the bibliography.
 *
 * @param array $form
 * @param array $form_state
 */
function islandora_bibliography_form_remove(array $form, array &$form_state) {
  module_load_include('inc', 'islandora_bibliography', 'Bibliography');
  $selections = array_filter($form_state['values']['table']['selections']);
  $pids = array_keys($selections);
  Bibliography::RemoveCitations($pids);
  drupal_set_message(t('Removed %num Citation(s)', array('%num' => count($pids))));
}

/**
 * Exports the selected pids to file.
 *
 * @param array $form
 * @param array $form_state
 */
function islandora_bibliography_form_export(array $form, array &$form_state) {
  module_load_include('inc', 'citation_exporter', 'Batch');
  $selections = array_filter($form_state['values']['table']['selections']);
  if ($selections != NULL) {
    $pids = array_keys($selections);
    if ($form_state['values']['format'] == 'HTML') {
      islandora_bibliography_form_preview($form, $form_state);
    }
    else {
      citation_exporter_batch_export($pids, $form_state['values']['style'], $form_state['values']['format']);
      batch_process();
    }
  }
  else {
    drupal_set_message(t('No citations were selected!'), 'warning');
  }
}

/**
 * Gets the MOD from an object.
 *
 * Helper function for getting the MODS metadata.
 *
 * @param string $pid
 *   The fedora object to get the MODS from.
 * 
 * @return DOMDocument
 *   An MODS document if found and non-empty, FALSE otherwise.
 */
function islandora_bibilgraphy_form_get_mods($pid) {
  module_load_include('inc', 'fedora_repository', 'api/fedora_item');
  $item = new Fedora_Item($pid);
  if ($item->exists() && isset($item->datastreams['MODS'])) {
    $mods = $item->get_datastream_dissemination('MODS');
    $mods = trim($mods);
    if (!empty($mods)) {
      $doc = new DOMDocument();
      $doc->loadXML($mods);
      return $doc;
    }
  }
  return FALSE;
}

/**
 * Theme's a form table for this module.
 *
 * @param array $element
 *   A Drupal Form Element.
 *
 * @return sting
 *   HTML that renders a table of settings for datastreams.
 */
function theme_islandora_bibliography_form_table(array $element) {
  $rows = array();
  foreach (element_children($element['rows']) as $child) {
    $setting = $element['rows'][$child];
    $pid = $setting['#pid'];
    $fields = array(
      drupal_render($element['selections'][$pid]) // First field is a checkbox
    );
    foreach (element_children($setting) as $property) {
      $field = $setting[$property];
      $fields[] = drupal_render($field);
    }
    $rows[] = array(
      'data' => $fields,
      'class' => isset($setting['#attributes']['class']) ? $setting['#attributes']['class'] : NULL
    );
  }
  $attributes = isset($element['#id']) ? array('id' => $element['#id']) : NULL;
  return theme_table($element['#header'], $rows, $attributes);
}