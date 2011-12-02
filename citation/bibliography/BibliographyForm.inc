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
        '#title' => t('My Citations'),
        '#description' => t('<p>Here you can remove or export citations from your bibliography.</p>'),
        'table' => islandora_bibilgraphy_form_table($pids),
        'preview_title' => array(
          '#value' => t('Preview Citations'),
          '#prefix' => '<h4>',
          '#suffix' => '</h4>'
        ),
        'preview_style' => array(
          '#title' => t('Style'),
          '#type' => 'select',
          '#options' => $styles
        ),
        'preview' => array(
          '#type' => 'submit',
          '#value' => t('Preview Selected'),
          '#submit' => array('islandora_bibliography_form_preview')
        ),
        'remove' => array(
          '#type' => 'submit',
          '#value' => t('Remove Selected'),
          '#submit' => array('islandora_bibliography_form_remove')
        ),
        'export_title' => array(
          '#value' => t('Export To File'),
          '#prefix' => '<h4>',
          '#suffix' => '</h4>'
        ),
        'format' => array(
          '#title' => t('Format'),
          '#type' => 'select',
          '#length' => 50,
          '#options' => array(
            'RIS' => 'RIS',
            'RTF' => 'RTF',
            'PDF' => 'PDF',
          )
        ),
        'style' => array(
          '#title' => t('Style'),
          '#type' => 'select',
          '#options' => $styles
        ),
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
    '#header' => array(theme('table_select_header_cell'), t('Title'), t('Author(s)'), ''),
    '#theme' => THEME_BIBLIOGRAPHY_FORM_TABLE,
    '#tree' => TRUE,
    'rows' => array(),
    'selections' => array(
      '#type' => 'checkboxes',
      '#options' => array_fill_keys($pids, ''),
    ),
  );
  $rows = &$table['rows'];
  foreach ($pids as $pid) {
    list($title, $authors) = islandora_bibilgraphy_form_get_title_authors($pid);
    $rows[] = array(
      '#pid' => $pid,
      'title' => array('#value' => check_plain($title)),
      'author' => array('#value' => check_plain($authors)),
      'remove' => array(
        '#type' => 'submit',
        '#value' => t('Remove'),
        '#pid' => $pid,
        '#submit' => array('islandora_bibliography_form_remove_citation_submit')
      )
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
    $authors = implode(' ', $mods->xpath('/mods:mods/mods:name/mods:role[mods:roleTerm = "author"]/../mods:namePart[@type="family"]'));
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
  $pid = $form_state['clicked_button']['#pid'];
  Bibliography::RemoveCitation($pid);
  drupal_set_message(t('Removed Citation'));
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
  $style = citeproc_style($form_state['values']['preview_style']);
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
  $output = $bibliography->render();
  $output .= l(t('Return to the Bibliography.'), 'bibliography');
  print theme('page', $output, FALSE);
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
  $pids = array_keys($selections);
  citation_exporter_batch_export($pids, $form_state['values']['style'], $form_state['values']['format']);
  batch_process();
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