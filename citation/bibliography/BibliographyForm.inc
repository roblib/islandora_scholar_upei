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
<head> 
<script type='text/javascript; e4x=1' src='/sites/default/modules/islandora_scholar_upei/citation/citeproc/lib/citeproc-js/xmle4x.js'></script><link type="text/css" rel="stylesheet" media="all" href="/sites/default/modules/islandora_scholar_upei/citation/bibliography/css/bibliography.css?3" />
<link type="text/css" rel="stylesheet" media="all" href="/sites/default/modules/islandora_scholar_upei/citation/bibliography/css/export.css?3" />
<link type="text/css" rel="stylesheet" media="all" href="/sites/default/modules/islandora_scholar_upei/citation/citeproc/css/citeproc.css?3" />
<script type="text/javascript" src="/sites/default/modules/islandora_scholar_upei/citation/citeproc/lib/citeproc-js/xmldom.js?3"></script>
<script type="text/javascript" src="/sites/default/modules/islandora_scholar_upei/citation/citeproc/lib/citeproc-js/citeproc.js?3"></script>
<script type="text/javascript" src="/sites/default/modules/islandora_scholar_upei/citation/citeproc/js/loadsys.js?3"></script>
<script type="text/javascript" src="/sites/default/modules/islandora_scholar_upei/citation/citeproc/js/runcites.js?3"></script>

<script type="text/javascript">
<!--//--><![CDATA[//><!--
jQuery.extend(Drupal.settings, { "basePath": "/", "citeproc": { "locale": { "en-US": "\x3c?xml version=\"1.0\" encoding=\"utf-8\"?\x3e\n\x3clocale xmlns=\"http://purl.org/net/xbiblio/csl\" version=\"1.0\" xml:lang=\"en-US\"\x3e\n  \x3cstyle-options punctuation-in-quote=\"true\"/\x3e\n  \x3cdate form=\"text\"\x3e\n    \x3cdate-part name=\"month\" suffix=\" \"/\x3e\n    \x3cdate-part name=\"day\" form=\"numeric-leading-zeros\" suffix=\", \"/\x3e\n    \x3cdate-part name=\"year\"/\x3e\n  \x3c/date\x3e\n  \x3cdate form=\"numeric\"\x3e\n    \x3cdate-part name=\"month\" form=\"numeric-leading-zeros\" suffix=\"/\"/\x3e\n    \x3cdate-part name=\"day\" form=\"numeric-leading-zeros\" suffix=\"/\"/\x3e\n    \x3cdate-part name=\"year\"/\x3e\n  \x3c/date\x3e\n  \x3cterms\x3e\n    \x3cterm name=\"accessed\"\x3eaccessed\x3c/term\x3e\n    \x3cterm name=\"and\"\x3eand\x3c/term\x3e\n    \x3cterm name=\"and others\"\x3eand others\x3c/term\x3e\n    \x3cterm name=\"anonymous\"\x3eanonymous\x3c/term\x3e\n    \x3cterm name=\"anonymous\" form=\"short\"\x3eanon.\x3c/term\x3e\n    \x3cterm name=\"at\"\x3eat\x3c/term\x3e\n    \x3cterm name=\"by\"\x3eby\x3c/term\x3e\n    \x3cterm name=\"circa\"\x3ecirca\x3c/term\x3e\n    \x3cterm name=\"circa\" form=\"short\"\x3ec.\x3c/term\x3e\n    \x3cterm name=\"cited\"\x3ecited\x3c/term\x3e\n    \x3cterm name=\"edition\"\x3e\n      \x3csingle\x3eedition\x3c/single\x3e\n      \x3cmultiple\x3eeditions\x3c/multiple\x3e\n    \x3c/term\x3e\n    \x3cterm name=\"edition\" form=\"short\"\x3eed.\x3c/term\x3e\n    \x3cterm name=\"et-al\"\x3eet al.\x3c/term\x3e\n    \x3cterm name=\"forthcoming\"\x3eforthcoming\x3c/term\x3e\n    \x3cterm name=\"from\"\x3efrom\x3c/term\x3e\n    \x3cterm name=\"ibid\"\x3eibid.\x3c/term\x3e\n    \x3cterm name=\"in\"\x3ein\x3c/term\x3e\n    \x3cterm name=\"in press\"\x3ein press\x3c/term\x3e\n    \x3cterm name=\"internet\"\x3einternet\x3c/term\x3e\n    \x3cterm name=\"interview\"\x3einterview\x3c/term\x3e\n    \x3cterm name=\"letter\"\x3eletter\x3c/term\x3e\n    \x3cterm name=\"no date\"\x3eno date\x3c/term\x3e\n    \x3cterm name=\"no date\" form=\"short\"\x3en.d.\x3c/term\x3e\n    \x3cterm name=\"online\"\x3eonline\x3c/term\x3e\n    \x3cterm name=\"presented at\"\x3epresented at the\x3c/term\x3e\n    \x3cterm name=\"reference\"\x3e\n      \x3csingle\x3ereference\x3c/single\x3e\n      \x3cmultiple\x3ereferences\x3c/multiple\x3e\n    \x3c/term\x3e\n    \x3cterm name=\"reference\" form=\"short\"\x3e\n      \x3csingle\x3eref.\x3c/single\x3e\n      \x3cmultiple\x3erefs.\x3c/multiple\x3e\n    \x3c/term\x3e\n    \x3cterm name=\"retrieved\"\x3eretrieved\x3c/term\x3e\n\n    \x3c!-- ANNO DOMINI; BEFORE CHRIST --\x3e\n    \x3cterm name=\"ad\"\x3eAD\x3c/term\x3e\n    \x3cterm name=\"bc\"\x3eBC\x3c/term\x3e\n\n    \x3c!-- QUOTES --\x3e\n    \x3cterm name=\"open-quote\"\x3e“\x3c/term\x3e\n    \x3cterm name=\"close-quote\"\x3e”\x3c/term\x3e\n    \x3cterm name=\"open-inner-quote\"\x3e‘\x3c/term\x3e\n    \x3cterm name=\"close-inner-quote\"\x3e’\x3c/term\x3e\n\n    \x3c!-- ORDINALS --\x3e\n    \x3cterm name=\"ordinal-01\"\x3est\x3c/term\x3e\n    \x3cterm name=\"ordinal-02\"\x3end\x3c/term\x3e\n    \x3cterm name=\"ordinal-03\"\x3erd\x3c/term\x3e\n    \x3cterm name=\"ordinal-04\"\x3eth\x3c/term\x3e\n\n    \x3c!-- LONG ORDINALS --\x3e\n    \x3cterm name=\"long-ordinal-01\"\x3efirst\x3c/term\x3e\n    \x3cterm name=\"long-ordinal-02\"\x3esecond\x3c/term\x3e\n    \x3cterm name=\"long-ordinal-03\"\x3ethird\x3c/term\x3e\n    \x3cterm name=\"long-ordinal-04\"\x3efourth\x3c/term\x3e\n    \x3cterm name=\"long-ordinal-05\"\x3efifth\x3c/term\x3e\n    \x3cterm name=\"long-ordinal-06\"\x3esixth\x3c/term\x3e\n    \x3cterm name=\"long-ordinal-07\"\x3eseventh\x3c/term\x3e\n    \x3cterm name=\"long-ordinal-08\"\x3eeighth\x3c/term\x3e\n    \x3cterm name=\"long-ordinal-09\"\x3eninth\x3c/term\x3e\n    \x3cterm name=\"long-ordinal-10\"\x3etenth\x3c/term\x3e\n\n    \x3c!-- CATEGORIES --\x3e\n    \x3cterm name=\"anthropology\"\x3eanthropology\x3c/term\x3e\n    \x3cterm name=\"astronomy\"\x3eastronomy\x3c/term\x3e\n    \x3cterm name=\"biology\"\x3ebiology\x3c/term\x3e\n    \x3cterm name=\"botany\"\x3ebotany\x3c/term\x3e\n    \x3cterm name=\"chemistry\"\x3echemistry\x3c/term\x3e\n    \x3cterm name=\"engineering\"\x3eengineering\x3c/term\x3e\n    \x3cterm name=\"generic-base\"\x3egeneric base\x3c/term\x3e\n    \x3cterm name=\"geography\"\x3egeography\x3c/term\x3e\n    \x3cterm name=\"geology\"\x3egeology\x3c/term\x3e\n    \x3cterm name=\"history\"\x3ehistory\x3c/term\x3e\n    \x3cterm name=\"humanities\"\x3ehumanities\x3c/term\x3e\n    \x3cterm name=\"linguistics\"\x3elinguistics\x3c/term\x3e\n    \x3cterm name=\"literature\"\x3eliterature\x3c/term\x3e\n    \x3cterm name=\"math\"\x3emath\x3c/term\x3e\n    \x3cterm name=\"medicine\"\x3emedicine\x3c/term\x3e\n    \x3cterm name=\"philosophy\"\x3ephilosophy\x3c/term\x3e\n    \x3cterm name=\"physics\"\x3ephysics\x3c/term\x3e\n    \x3cterm name=\"psychology\"\x3epsychology\x3c/term\x3e\n    \x3cterm name=\"sociology\"\x3esociology\x3c/term\x3e\n    \x3cterm name=\"science\"\x3escience\x3c/term\x3e\n    \x3cterm name=\"political_science\"\x3epolitical science\x3c/term\x3e\n    \x3cterm name=\"social_science\"\x3esocial science\x3c/term\x3e\n    \x3cterm name=\"theology\"\x3etheology\x3c/term\x3e\n    \x3cterm name=\"zoology\"\x3ezoology\x3c/term\x3e\n\n    \x3c!-- LONG LOCATOR FORMS --\x3e\n    \x3cterm name=\"book\"\x3e\n      \x3csingle\x3ebook\x3c/single\x3e\n      \x3cmultiple\x3ebooks\x3c/multiple\x3e\n    \x3c/term\x3e\n    \x3cterm name=\"chapter\"\x3e\n      \x3csingle\x3echapter\x3c/single\x3e\n      \x3cmultiple\x3echapters\x3c/multiple\x3e\n    \x3c/term\x3e\n    \x3cterm name=\"column\"\x3e\n      \x3csingle\x3ecolumn\x3c/single\x3e\n      \x3cmultiple\x3ecolumns\x3c/multiple\x3e\n    \x3c/term\x3e\n    \x3cterm name=\"figure\"\x3e\n      \x3csingle\x3efigure\x3c/single\x3e\n      \x3cmultiple\x3efigures\x3c/multiple\x3e\n    \x3c/term\x3e\n    \x3cterm name=\"folio\"\x3e\n      \x3csingle\x3efolio\x3c/single\x3e\n      \x3cmultiple\x3efolios\x3c/multiple\x3e\n    \x3c/term\x3e\n    \x3cterm name=\"issue\"\x3e\n      \x3csingle\x3enumber\x3c/single\x3e\n      \x3cmultiple\x3enumbers\x3c/multiple\x3e\n    \x3c/term\x3e\n    \x3cterm name=\"line\"\x3e\n      \x3csingle\x3eline\x3c/single\x3e\n      \x3cmultiple\x3elines\x3c/multiple\x3e\n    \x3c/term\x3e\n    \x3cterm name=\"note\"\x3e\n      \x3csingle\x3enote\x3c/single\x3e\n      \x3cmultiple\x3enotes\x3c/multiple\x3e\n    \x3c/term\x3e\n    \x3cterm name=\"opus\"\x3e\n      \x3csingle\x3eopus\x3c/single\x3e\n      \x3cmultiple\x3eopera\x3c/multiple\x3e\n    \x3c/term\x3e\n    \x3cterm name=\"page\"\x3e\n      \x3csingle\x3epage\x3c/single\x3e\n      \x3cmultiple\x3epages\x3c/multiple\x3e\n    \x3c/term\x3e\n    \x3cterm name=\"paragraph\"\x3e\n      \x3csingle\x3eparagraph\x3c/single\x3e\n      \x3cmultiple\x3eparagraph\x3c/multiple\x3e\n    \x3c/term\x3e\n    \x3cterm name=\"part\"\x3e\n      \x3csingle\x3epart\x3c/single\x3e\n      \x3cmultiple\x3eparts\x3c/multiple\x3e\n    \x3c/term\x3e\n    \x3cterm name=\"section\"\x3e\n      \x3csingle\x3esection\x3c/single\x3e\n      \x3cmultiple\x3esections\x3c/multiple\x3e\n    \x3c/term\x3e\n    \x3cterm name=\"sub verbo\"\x3e\n      \x3csingle\x3esub verbo\x3c/single\x3e\n      \x3cmultiple\x3esub verbis\x3c/multiple\x3e\n    \x3c/term\x3e\n    \x3cterm name=\"verse\"\x3e\n      \x3csingle\x3everse\x3c/single\x3e\n      \x3cmultiple\x3everses\x3c/multiple\x3e\n    \x3c/term\x3e\n    \x3cterm name=\"volume\"\x3e\n      \x3csingle\x3evolume\x3c/single\x3e\n      \x3cmultiple\x3evolumes\x3c/multiple\x3e\n    \x3c/term\x3e\n\n    \x3c!-- SHORT LOCATOR FORMS --\x3e\n    \x3cterm name=\"book\" form=\"short\"\x3ebk.\x3c/term\x3e\n    \x3cterm name=\"chapter\" form=\"short\"\x3echap.\x3c/term\x3e\n    \x3cterm name=\"column\" form=\"short\"\x3ecol.\x3c/term\x3e\n    \x3cterm name=\"figure\" form=\"short\"\x3efig.\x3c/term\x3e\n    \x3cterm name=\"folio\" form=\"short\"\x3ef.\x3c/term\x3e\n    \x3cterm name=\"issue\" form=\"short\"\x3eno.\x3c/term\x3e\n    \x3cterm name=\"opus\" form=\"short\"\x3eop.\x3c/term\x3e\n    \x3cterm name=\"page\" form=\"short\"\x3e\n      \x3csingle\x3ep.\x3c/single\x3e\n      \x3cmultiple\x3epp.\x3c/multiple\x3e\n    \x3c/term\x3e\n    \x3cterm name=\"paragraph\" form=\"short\"\x3epara.\x3c/term\x3e\n    \x3cterm name=\"part\" form=\"short\"\x3ept.\x3c/term\x3e\n    \x3cterm name=\"section\" form=\"short\"\x3esec.\x3c/term\x3e\n    \x3cterm name=\"sub verbo\" form=\"short\"\x3e\n      \x3csingle\x3es.v.\x3c/single\x3e\n      \x3cmultiple\x3es.vv.\x3c/multiple\x3e\n    \x3c/term\x3e\n    \x3cterm name=\"verse\" form=\"short\"\x3e\n      \x3csingle\x3ev.\x3c/single\x3e\n      \x3cmultiple\x3evv.\x3c/multiple\x3e\n    \x3c/term\x3e\n    \x3cterm name=\"volume\" form=\"short\"\x3e\n      \x3csingle\x3evol.\x3c/single\x3e\n      \x3cmultiple\x3evols.\x3c/multiple\x3e\n    \x3c/term\x3e\n\n    \x3c!-- SYMBOL LOCATOR FORMS --\x3e\n    \x3cterm name=\"paragraph\" form=\"symbol\"\x3e\n      \x3csingle\x3e¶\x3c/single\x3e\n      \x3cmultiple\x3e¶¶\x3c/multiple\x3e\n    \x3c/term\x3e\n    \x3cterm name=\"section\" form=\"symbol\"\x3e\n      \x3csingle\x3e§\x3c/single\x3e\n      \x3cmultiple\x3e§§\x3c/multiple\x3e\n    \x3c/term\x3e\n\n    \x3c!-- LONG ROLE FORMS --\x3e\n    \x3cterm name=\"author\"\x3e\n      \x3csingle/\x3e\n      \x3cmultiple/\x3e\n    \x3c/term\x3e\n    \x3cterm name=\"editor\"\x3e\n      \x3csingle\x3eeditor\x3c/single\x3e\n      \x3cmultiple\x3eeditors\x3c/multiple\x3e\n    \x3c/term\x3e\n    \x3cterm name=\"editorial-director\"\x3e\n      \x3csingle\x3eeditor\x3c/single\x3e\n      \x3cmultiple\x3eeditors\x3c/multiple\x3e\n    \x3c/term\x3e\n    \x3cterm name=\"translator\"\x3e\n      \x3csingle\x3etranslator\x3c/single\x3e\n      \x3cmultiple\x3etranslators\x3c/multiple\x3e\n    \x3c/term\x3e\n    \x3cterm name=\"editortranslator\"\x3e\n      \x3csingle\x3eeditor \x26amp; translator\x3c/single\x3e\n      \x3cmultiple\x3eeditors \x26amp; translators\x3c/multiple\x3e\n    \x3c/term\x3e\n\n    \x3c!-- SHORT ROLE FORMS --\x3e\n    \x3cterm name=\"author\" form=\"short\"\x3e\n      \x3csingle/\x3e\n      \x3cmultiple/\x3e\n    \x3c/term\x3e\n    \x3cterm name=\"editor\" form=\"short\"\x3e\n      \x3csingle\x3eed.\x3c/single\x3e\n      \x3cmultiple\x3eeds.\x3c/multiple\x3e\n    \x3c/term\x3e\n    \x3cterm name=\"editorial-director\" form=\"short\"\x3e\n      \x3csingle\x3eed.\x3c/single\x3e\n      \x3cmultiple\x3eeds.\x3c/multiple\x3e\n    \x3c/term\x3e\n    \x3cterm name=\"translator\" form=\"short\"\x3e\n      \x3csingle\x3etran.\x3c/single\x3e\n      \x3cmultiple\x3etrans.\x3c/multiple\x3e\n    \x3c/term\x3e\n    \x3cterm name=\"editortranslator\" form=\"short\"\x3e\n      \x3csingle\x3eed. \x26amp; tran.\x3c/single\x3e\n      \x3cmultiple\x3eeds. \x26amp; trans.\x3c/multiple\x3e\n    \x3c/term\x3e\n\n    \x3c!-- VERB ROLE FORMS --\x3e\n    \x3cterm name=\"editor\" form=\"verb\"\x3eedited by\x3c/term\x3e\n    \x3cterm name=\"editorial-director\" form=\"verb\"\x3eedited by\x3c/term\x3e\n    \x3cterm name=\"translator\" form=\"verb\"\x3etranslated by\x3c/term\x3e\n    \x3cterm name=\"editortranslator\" form=\"verb\"\x3eedited \x26amp; translated by\x3c/term\x3e\n    \x3cterm name=\"recipient\" form=\"verb\"\x3eto\x3c/term\x3e\n    \x3cterm name=\"interviewer\" form=\"verb\"\x3einterview by\x3c/term\x3e\n\n    \x3c!-- SHORT VERB ROLE FORMS --\x3e\n    \x3cterm name=\"container-author\" form=\"verb-short\"\x3eby\x3c/term\x3e\n    \x3cterm name=\"editor\" form=\"verb-short\"\x3eed.\x3c/term\x3e\n    \x3cterm name=\"editorial-director\" form=\"verb-short\"\x3eed.\x3c/term\x3e\n    \x3cterm name=\"translator\" form=\"verb-short\"\x3etrans.\x3c/term\x3e\n    \x3cterm name=\"editortranslator\" form=\"verb-short\"\x3eed. \x26amp; trans. by\x3c/term\x3e\n\n    \x3c!-- LONG MONTH FORMS --\x3e\n    \x3cterm name=\"month-01\"\x3eJanuary\x3c/term\x3e\n    \x3cterm name=\"month-02\"\x3eFebruary\x3c/term\x3e\n    \x3cterm name=\"month-03\"\x3eMarch\x3c/term\x3e\n    \x3cterm name=\"month-04\"\x3eApril\x3c/term\x3e\n    \x3cterm name=\"month-05\"\x3eMay\x3c/term\x3e\n    \x3cterm name=\"month-06\"\x3eJune\x3c/term\x3e\n    \x3cterm name=\"month-07\"\x3eJuly\x3c/term\x3e\n    \x3cterm name=\"month-08\"\x3eAugust\x3c/term\x3e\n    \x3cterm name=\"month-09\"\x3eSeptember\x3c/term\x3e\n    \x3cterm name=\"month-10\"\x3eOctober\x3c/term\x3e\n    \x3cterm name=\"month-11\"\x3eNovember\x3c/term\x3e\n    \x3cterm name=\"month-12\"\x3eDecember\x3c/term\x3e\n\n    \x3c!-- SHORT MONTH FORMS --\x3e\n    \x3cterm name=\"month-01\" form=\"short\"\x3eJan.\x3c/term\x3e\n    \x3cterm name=\"month-02\" form=\"short\"\x3eFeb.\x3c/term\x3e\n    \x3cterm name=\"month-03\" form=\"short\"\x3eMar.\x3c/term\x3e\n    \x3cterm name=\"month-04\" form=\"short\"\x3eApr.\x3c/term\x3e\n    \x3cterm name=\"month-05\" form=\"short\"\x3eMay\x3c/term\x3e\n    \x3cterm name=\"month-06\" form=\"short\"\x3eJun.\x3c/term\x3e\n    \x3cterm name=\"month-07\" form=\"short\"\x3eJul.\x3c/term\x3e\n    \x3cterm name=\"month-08\" form=\"short\"\x3eAug.\x3c/term\x3e\n    \x3cterm name=\"month-09\" form=\"short\"\x3eSep.\x3c/term\x3e\n    \x3cterm name=\"month-10\" form=\"short\"\x3eOct.\x3c/term\x3e\n    \x3cterm name=\"month-11\" form=\"short\"\x3eNov.\x3c/term\x3e\n    \x3cterm name=\"month-12\" form=\"short\"\x3eDec.\x3c/term\x3e\n\n    \x3c!-- SEASONS --\x3e\n    \x3cterm name=\"season-01\"\x3eSpring\x3c/term\x3e\n    \x3cterm name=\"season-02\"\x3eSummer\x3c/term\x3e\n    \x3cterm name=\"season-03\"\x3eAutumn\x3c/term\x3e\n    \x3cterm name=\"season-04\"\x3eWinter\x3c/term\x3e\n  \x3c/terms\x3e\n\x3c/locale\x3e\n" }, "style": { "4fa907c35f6c9": "\x3c?xml version=\"1.0\" encoding=\"utf-8\"?\x3e\n\x3cstyle xmlns=\"http://purl.org/net/xbiblio/csl\" class=\"in-text\" version=\"1.0\" demote-non-dropping-particle=\"never\"\x3e\n  \x3cinfo\x3e\n    \x3ctitle\x3eAmerican Psychological Association 6th Edition\x3c/title\x3e\n    \x3cid\x3ehttp://www.zotero.org/styles/apa\x3c/id\x3e\n    \x3clink href=\"http://www.zotero.org/styles/apa\" rel=\"self\"/\x3e\n    \x3clink href=\"http://owl.english.purdue.edu/owl/resource/560/01/\" rel=\"documentation\"/\x3e\n    \x3cauthor\x3e\n      \x3cname\x3eSimon Kornblith\x3c/name\x3e\n      \x3cemail\x3esimon@simonster.com\x3c/email\x3e\n    \x3c/author\x3e\n    \x3ccontributor\x3e\n      \x3cname\x3eBruce D\'Arcus\x3c/name\x3e\n    \x3c/contributor\x3e\n    \x3ccontributor\x3e\n      \x3cname\x3eCurtis M. Humphrey\x3c/name\x3e\n    \x3c/contributor\x3e\n    \x3ccontributor\x3e\n      \x3cname\x3eRichard Karnesky\x3c/name\x3e\n      \x3cemail\x3ekarnesky+zotero@gmail.com\x3c/email\x3e\n      \x3curi\x3ehttp://arc.nucapt.northwestern.edu/Richard_Karnesky\x3c/uri\x3e\n    \x3c/contributor\x3e\n    \x3ccontributor\x3e\n      \x3cname\x3eSebastian Karcher\x3c/name\x3e\n    \x3c/contributor\x3e\n    \x3ccategory field=\"psychology\"/\x3e\n    \x3ccategory field=\"generic-base\"/\x3e\n    \x3ccategory citation-format=\"author-date\"/\x3e\n    \x3cupdated\x3e2012-03-01T23:01:02+00:00\x3c/updated\x3e\n    \x3crights\x3eThis work is licensed under a Creative Commons Attribution-ShareAlike 3.0 License: http://creativecommons.org/licenses/by-sa/3.0/\x3c/rights\x3e\n  \x3c/info\x3e\n  \x3clocale xml:lang=\"en\"\x3e\n    \x3cterms\x3e\n      \x3cterm name=\"translator\" form=\"short\"\x3e\n        \x3csingle\x3etrans.\x3c/single\x3e\n        \x3cmultiple\x3etrans.\x3c/multiple\x3e\n      \x3c/term\x3e\n    \x3c/terms\x3e\n  \x3c/locale\x3e\n  \x3cmacro name=\"container-contributors\"\x3e\n    \x3cchoose\x3e\n      \x3cif type=\"chapter paper-conference\" match=\"any\"\x3e\n        \x3ctext term=\"in\" text-case=\"capitalize-first\" suffix=\" \"/\x3e\n        \x3cnames variable=\"editor\" delimiter=\", \" suffix=\", \"\x3e\n          \x3cname and=\"symbol\" initialize-with=\". \" delimiter=\", \"/\x3e\n          \x3clabel form=\"short\" prefix=\" (\" text-case=\"capitalize-first\" suffix=\")\"/\x3e\n          \x3csubstitute\x3e\n            \x3cnames variable=\"translator\"/\x3e\n          \x3c/substitute\x3e\n        \x3c/names\x3e\n      \x3c/if\x3e\n    \x3c/choose\x3e\n  \x3c/macro\x3e\n  \x3cmacro name=\"secondary-contributors\"\x3e\n    \x3cchoose\x3e\n      \x3cif type=\"chapter paper-conference\" match=\"none\"\x3e\n        \x3cnames variable=\"translator\" delimiter=\", \" prefix=\" (\" suffix=\")\"\x3e\n          \x3cname and=\"symbol\" initialize-with=\". \" delimiter=\", \"/\x3e\n          \x3clabel form=\"short\" prefix=\", \" text-case=\"capitalize-first\" suffix=\"\"/\x3e\n          \x3csubstitute\x3e\n            \x3cnames variable=\"editor\"/\x3e\n          \x3c/substitute\x3e\n        \x3c/names\x3e\n      \x3c/if\x3e\n    \x3c/choose\x3e\n  \x3c/macro\x3e\n  \x3cmacro name=\"author\"\x3e\n    \x3cnames variable=\"author\"\x3e\n      \x3cname name-as-sort-order=\"all\" and=\"symbol\" sort-separator=\", \" initialize-with=\". \" delimiter=\", \" delimiter-precedes-last=\"always\"/\x3e\n      \x3clabel form=\"short\" prefix=\" (\" suffix=\".)\" text-case=\"capitalize-first\" strip-periods=\"true\"/\x3e\n      \x3csubstitute\x3e\n        \x3cnames variable=\"editor\"/\x3e\n        \x3cnames variable=\"translator\"/\x3e\n        \x3cchoose\x3e\n          \x3cif type=\"report\"\x3e\n            \x3ctext variable=\"publisher\"/\x3e\n            \x3ctext macro=\"title\"/\x3e\n          \x3c/if\x3e\n          \x3celse\x3e\n            \x3ctext macro=\"title\"/\x3e\n          \x3c/else\x3e\n        \x3c/choose\x3e\n      \x3c/substitute\x3e\n    \x3c/names\x3e\n  \x3c/macro\x3e\n  \x3cmacro name=\"author-short\"\x3e\n    \x3cnames variable=\"author\"\x3e\n      \x3cname form=\"short\" and=\"symbol\" delimiter=\", \" initialize-with=\". \"/\x3e\n      \x3csubstitute\x3e\n        \x3cnames variable=\"editor\"/\x3e\n        \x3cnames variable=\"translator\"/\x3e\n        \x3cchoose\x3e\n          \x3cif type=\"report\"\x3e\n            \x3ctext variable=\"publisher\"/\x3e\n            \x3ctext variable=\"title\" form=\"short\" font-style=\"italic\"/\x3e\n          \x3c/if\x3e\n          \x3celse-if type=\"bill book graphic legal_case motion_picture song\" match=\"any\"\x3e\n            \x3ctext variable=\"title\" form=\"short\" font-style=\"italic\"/\x3e\n          \x3c/else-if\x3e\n          \x3celse\x3e\n            \x3ctext variable=\"title\" form=\"short\" quotes=\"true\"/\x3e\n          \x3c/else\x3e\n        \x3c/choose\x3e\n      \x3c/substitute\x3e\n    \x3c/names\x3e\n  \x3c/macro\x3e\n  \x3cmacro name=\"access\"\x3e\n    \x3cchoose\x3e\n      \x3cif type=\"thesis\"\x3e\n        \x3cchoose\x3e\n          \x3cif variable=\"archive\" match=\"any\"\x3e\n            \x3cgroup\x3e\n              \x3ctext term=\"retrieved\" text-case=\"capitalize-first\" suffix=\" \"/\x3e\n              \x3ctext term=\"from\" suffix=\" \"/\x3e\n              \x3ctext variable=\"archive\" suffix=\".\"/\x3e\n              \x3ctext variable=\"archive_location\" prefix=\" (\" suffix=\")\"/\x3e\n            \x3c/group\x3e\n          \x3c/if\x3e\n          \x3celse\x3e\n            \x3cgroup\x3e\n              \x3ctext term=\"retrieved\" text-case=\"capitalize-first\" suffix=\" \"/\x3e\n              \x3ctext term=\"from\" suffix=\" \"/\x3e\n              \x3ctext variable=\"URL\"/\x3e\n            \x3c/group\x3e\n          \x3c/else\x3e\n        \x3c/choose\x3e\n      \x3c/if\x3e\n      \x3celse\x3e\n        \x3cchoose\x3e\n          \x3cif variable=\"DOI\"\x3e\n            \x3ctext variable=\"DOI\" prefix=\"doi:\"/\x3e\n          \x3c/if\x3e\n          \x3celse\x3e\n            \x3cchoose\x3e\n              \x3cif type=\"webpage\"\x3e\n                \x3cgroup delimiter=\" \"\x3e\n                  \x3ctext term=\"retrieved\" text-case=\"capitalize-first\" suffix=\" \"/\x3e\n                  \x3cgroup\x3e\n                    \x3cdate variable=\"accessed\" suffix=\", \"\x3e\n                      \x3cdate-part name=\"month\" suffix=\" \"/\x3e\n                      \x3cdate-part name=\"day\" suffix=\", \"/\x3e\n                      \x3cdate-part name=\"year\"/\x3e\n                    \x3c/date\x3e\n                  \x3c/group\x3e\n                  \x3ctext term=\"from\"/\x3e\n                  \x3ctext variable=\"URL\"/\x3e\n                \x3c/group\x3e\n              \x3c/if\x3e\n              \x3celse\x3e\n                \x3cgroup\x3e\n                  \x3ctext term=\"retrieved\" text-case=\"capitalize-first\" suffix=\" \"/\x3e\n                  \x3ctext term=\"from\" suffix=\" \"/\x3e\n                  \x3ctext variable=\"URL\"/\x3e\n                \x3c/group\x3e\n              \x3c/else\x3e\n            \x3c/choose\x3e\n          \x3c/else\x3e\n        \x3c/choose\x3e\n      \x3c/else\x3e\n    \x3c/choose\x3e\n  \x3c/macro\x3e\n  \x3cmacro name=\"title\"\x3e\n    \x3cchoose\x3e\n      \x3cif type=\"report thesis\" match=\"any\"\x3e\n        \x3ctext variable=\"title\" font-style=\"italic\"/\x3e\n        \x3cgroup prefix=\" (\" suffix=\")\"\x3e\n          \x3ctext variable=\"genre\"/\x3e\n          \x3ctext variable=\"number\" prefix=\" No. \"/\x3e\n        \x3c/group\x3e\n      \x3c/if\x3e\n      \x3celse-if type=\"book graphic  motion_picture report song manuscript speech\" match=\"any\"\x3e\n        \x3ctext variable=\"title\" font-style=\"italic\"/\x3e\n      \x3c/else-if\x3e\n      \x3celse\x3e\n        \x3ctext variable=\"title\"/\x3e\n      \x3c/else\x3e\n    \x3c/choose\x3e\n  \x3c/macro\x3e\n  \x3cmacro name=\"publisher\"\x3e\n    \x3cchoose\x3e\n      \x3cif type=\"report\" match=\"any\"\x3e\n        \x3cgroup delimiter=\": \"\x3e\n          \x3ctext variable=\"publisher-place\"/\x3e\n          \x3ctext variable=\"publisher\"/\x3e\n        \x3c/group\x3e\n      \x3c/if\x3e\n      \x3celse-if type=\"thesis\" match=\"any\"\x3e\n        \x3cgroup delimiter=\", \"\x3e\n          \x3ctext variable=\"publisher\"/\x3e\n          \x3ctext variable=\"publisher-place\"/\x3e\n        \x3c/group\x3e\n      \x3c/else-if\x3e\n      \x3celse\x3e\n        \x3cgroup delimiter=\", \"\x3e\n          \x3cchoose\x3e\n            \x3cif variable=\"event\" match=\"none\"\x3e\n              \x3ctext variable=\"genre\"/\x3e\n            \x3c/if\x3e\n          \x3c/choose\x3e\n          \x3cgroup delimiter=\": \"\x3e\n            \x3ctext variable=\"publisher-place\"/\x3e\n            \x3ctext variable=\"publisher\"/\x3e\n          \x3c/group\x3e\n        \x3c/group\x3e\n      \x3c/else\x3e\n    \x3c/choose\x3e\n  \x3c/macro\x3e\n  \x3cmacro name=\"event\"\x3e\n    \x3cchoose\x3e\n      \x3cif variable=\"event\"\x3e\n        \x3cchoose\x3e\n          \x3cif variable=\"genre\" match=\"none\"\x3e\n            \x3ctext term=\"presented at\" text-case=\"capitalize-first\" suffix=\" \"/\x3e\n            \x3ctext variable=\"event\"/\x3e\n          \x3c/if\x3e\n          \x3celse\x3e\n            \x3cgroup delimiter=\" \"\x3e\n              \x3ctext variable=\"genre\" text-case=\"capitalize-first\"/\x3e\n              \x3ctext term=\"presented at\"/\x3e\n              \x3ctext variable=\"event\"/\x3e\n            \x3c/group\x3e\n          \x3c/else\x3e\n        \x3c/choose\x3e\n      \x3c/if\x3e\n    \x3c/choose\x3e\n  \x3c/macro\x3e\n  \x3cmacro name=\"issued\"\x3e\n    \x3cchoose\x3e\n      \x3cif type=\"legal_case bill\" match=\"none\"\x3e\n	\x3cchoose\x3e\n	  \x3cif variable=\"issued\"\x3e\n	    \x3cgroup prefix=\" (\" suffix=\").\"\x3e\n	      \x3cdate variable=\"issued\"\x3e\n		\x3cdate-part name=\"year\"/\x3e\n	      \x3c/date\x3e\n	      \x3ctext variable=\"year-suffix\"/\x3e\n	      \x3cchoose\x3e\n		\x3cif type=\"bill book graphic legal_case motion_picture report song article-journal chapter paper-conference\" match=\"none\"\x3e\n		  \x3cdate variable=\"issued\"\x3e\n		    \x3cdate-part prefix=\", \" name=\"month\"/\x3e\n		    \x3cdate-part prefix=\" \" name=\"day\"/\x3e\n		  \x3c/date\x3e\n		\x3c/if\x3e\n	      \x3c/choose\x3e\n	    \x3c/group\x3e\n	  \x3c/if\x3e\n	  \x3celse\x3e\n	    \x3cgroup prefix=\" (\" suffix=\").\"\x3e\n	      \x3ctext term=\"no date\" form=\"short\"/\x3e\n	      \x3ctext variable=\"year-suffix\" prefix=\"-\"/\x3e\n	    \x3c/group\x3e\n	  \x3c/else\x3e\n	\x3c/choose\x3e\n      \x3c/if\x3e\n    \x3c/choose\x3e\n  \x3c/macro\x3e\n  \x3cmacro name=\"issued-sort\"\x3e\n    \x3cchoose\x3e\n      \x3cif type=\"bill book graphic legal_case motion_picture report song article-journal chapter paper-conference\" match=\"none\"\x3e\n        \x3cdate variable=\"issued\"\x3e\n          \x3cdate-part name=\"year\"/\x3e\n          \x3cdate-part prefix=\", \" name=\"month\"/\x3e\n          \x3cdate-part prefix=\" \" name=\"day\"/\x3e\n        \x3c/date\x3e\n      \x3c/if\x3e\n      \x3celse\x3e\n        \x3cdate variable=\"issued\"\x3e\n          \x3cdate-part name=\"year\"/\x3e\n        \x3c/date\x3e\n      \x3c/else\x3e\n    \x3c/choose\x3e\n  \x3c/macro\x3e\n  \x3cmacro name=\"issued-year\"\x3e\n    \x3cchoose\x3e\n      \x3cif variable=\"issued\"\x3e\n        \x3cdate variable=\"issued\"\x3e\n          \x3cdate-part name=\"year\"/\x3e\n        \x3c/date\x3e\n        \x3ctext variable=\"year-suffix\"/\x3e\n      \x3c/if\x3e\n      \x3celse\x3e\n        \x3ctext term=\"no date\" form=\"short\"/\x3e\n        \x3ctext variable=\"year-suffix\" prefix=\"-\"/\x3e\n      \x3c/else\x3e\n    \x3c/choose\x3e\n  \x3c/macro\x3e\n  \x3cmacro name=\"edition\"\x3e\n    \x3cchoose\x3e\n      \x3cif is-numeric=\"edition\"\x3e\n        \x3cgroup delimiter=\" \"\x3e\n          \x3cnumber variable=\"edition\" form=\"ordinal\"/\x3e\n          \x3ctext term=\"edition\" form=\"short\" suffix=\".\" strip-periods=\"true\"/\x3e\n        \x3c/group\x3e\n      \x3c/if\x3e\n      \x3celse\x3e\n        \x3ctext variable=\"edition\" suffix=\".\"/\x3e\n      \x3c/else\x3e\n    \x3c/choose\x3e\n  \x3c/macro\x3e\n  \x3cmacro name=\"locators\"\x3e\n    \x3cchoose\x3e\n      \x3cif type=\"article-journal article-magazine\" match=\"any\"\x3e\n        \x3cgroup prefix=\", \" delimiter=\", \"\x3e\n          \x3cgroup\x3e\n            \x3ctext variable=\"volume\" font-style=\"italic\"/\x3e\n            \x3ctext variable=\"issue\" prefix=\"(\" suffix=\")\"/\x3e\n          \x3c/group\x3e\n          \x3ctext variable=\"page\"/\x3e\n        \x3c/group\x3e\n      \x3c/if\x3e\n      \x3celse-if type=\"article-newspaper\"\x3e\n        \x3cgroup delimiter=\" \" prefix=\", \"\x3e\n          \x3clabel variable=\"page\" form=\"short\"/\x3e\n          \x3ctext variable=\"page\"/\x3e\n        \x3c/group\x3e\n      \x3c/else-if\x3e\n      \x3celse-if type=\"book graphic motion_picture report song chapter paper-conference\" match=\"any\"\x3e\n        \x3cgroup prefix=\" (\" suffix=\")\" delimiter=\", \"\x3e\n          \x3ctext macro=\"edition\"/\x3e\n          \x3cgroup\x3e\n            \x3ctext term=\"volume\" form=\"short\" plural=\"true\" text-case=\"capitalize-first\" suffix=\". \" strip-periods=\"true\"/\x3e\n            \x3cnumber variable=\"number-of-volumes\" form=\"numeric\" prefix=\"1-\"/\x3e\n          \x3c/group\x3e\n          \x3cgroup\x3e\n            \x3ctext term=\"volume\" form=\"short\" text-case=\"capitalize-first\" suffix=\". \" strip-periods=\"true\"/\x3e\n            \x3cnumber variable=\"volume\" form=\"numeric\"/\x3e\n          \x3c/group\x3e\n          \x3cgroup\x3e\n            \x3clabel variable=\"page\" form=\"short\" suffix=\" \"/\x3e\n            \x3ctext variable=\"page\"/\x3e\n          \x3c/group\x3e\n        \x3c/group\x3e\n      \x3c/else-if\x3e\n      \x3celse-if type=\"legal_case\"\x3e\n	\x3cgroup prefix=\" (\" suffix=\")\" delimiter=\" \"\x3e\n	  \x3ctext variable=\"authority\"/\x3e\n	  \x3cdate variable=\"issued\" delimiter=\" \"\x3e\n	    \x3cdate-part name=\"month\" form=\"short\"/\x3e\n	    \x3cdate-part name=\"day\" suffix=\",\"/\x3e\n	    \x3cdate-part name=\"year\"/\x3e\n	  \x3c/date\x3e\n	\x3c/group\x3e\n      \x3c/else-if\x3e\n      \x3celse-if type=\"bill\"\x3e\n	\x3cdate variable=\"issued\" prefix=\" (\" suffix=\")\"\x3e\n	  \x3cdate-part name=\"year\"/\x3e\n	\x3c/date\x3e\n      \x3c/else-if\x3e\n    \x3c/choose\x3e\n  \x3c/macro\x3e\n  \x3cmacro name=\"citation-locator\"\x3e\n    \x3cgroup\x3e\n      \x3clabel variable=\"locator\" form=\"short\"/\x3e\n      \x3ctext variable=\"locator\" prefix=\" \"/\x3e\n    \x3c/group\x3e\n  \x3c/macro\x3e\n  \x3cmacro name=\"container\"\x3e\n    \x3cchoose\x3e\n      \x3cif type=\"legal_case bill\" match=\"none\"\x3e\n	\x3ctext variable=\"container-title\" font-style=\"italic\"/\x3e\n      \x3c/if\x3e\n      \x3celse\x3e\n	\x3cgroup delimiter=\" \" prefix=\", \"\x3e\n	  \x3cchoose\x3e\n	    \x3cif variable=\"container-title\"\x3e\n	      \x3ctext variable=\"volume\"/\x3e\n	      \x3ctext variable=\"container-title\"/\x3e\n	      \x3cgroup delimiter=\" \"\x3e\n		\x3c!--change to label variable=\"section\" as that becomes available --\x3e\n		\x3ctext term=\"section\" form=\"symbol\"/\x3e\n		\x3ctext variable=\"section\"/\x3e\n	      \x3c/group\x3e\n	      \x3ctext variable=\"page\"/\x3e\n	    \x3c/if\x3e\n	    \x3celse\x3e\n	      \x3cchoose\x3e\n		\x3cif type=\"legal_case\"\x3e\n		  \x3ctext variable=\"number\" prefix=\"No. \"/\x3e\n		\x3c/if\x3e\n		\x3celse\x3e\n		  \x3ctext variable=\"number\" prefix=\"Pub. L. No. \"/\x3e\n		  \x3cgroup delimiter=\" \"\x3e\n		    \x3c!--change to label variable=\"section\" as that becomes available --\x3e\n		    \x3ctext term=\"section\" form=\"symbol\"/\x3e\n		    \x3ctext variable=\"section\"/\x3e\n		  \x3c/group\x3e\n		\x3c/else\x3e\n	      \x3c/choose\x3e\n	    \x3c/else\x3e\n	  \x3c/choose\x3e\n	\x3c/group\x3e\n      \x3c/else\x3e\n    \x3c/choose\x3e\n  \x3c/macro\x3e\n  \x3ccitation et-al-min=\"6\" et-al-use-first=\"1\" et-al-subsequent-min=\"3\" et-al-subsequent-use-first=\"1\" disambiguate-add-year-suffix=\"true\" disambiguate-add-names=\"true\" disambiguate-add-givenname=\"true\" collapse=\"year\" givenname-disambiguation-rule=\"primary-name\"\x3e\n    \x3csort\x3e\n      \x3ckey macro=\"author\"/\x3e\n      \x3ckey macro=\"issued-sort\"/\x3e\n    \x3c/sort\x3e\n    \x3clayout prefix=\"(\" suffix=\")\" delimiter=\"; \"\x3e\n      \x3cgroup delimiter=\", \"\x3e\n        \x3ctext macro=\"author-short\"/\x3e\n        \x3ctext macro=\"issued-year\"/\x3e\n        \x3ctext macro=\"citation-locator\"/\x3e\n      \x3c/group\x3e\n    \x3c/layout\x3e\n  \x3c/citation\x3e\n  \x3cbibliography hanging-indent=\"true\" et-al-min=\"8\" et-al-use-first=\"7\" entry-spacing=\"0\" line-spacing=\"2\"\x3e\n    \x3csort\x3e\n      \x3ckey macro=\"author\"/\x3e\n      \x3ckey macro=\"issued-sort\" sort=\"ascending\"/\x3e\n    \x3c/sort\x3e\n    \x3clayout\x3e\n      \x3cgroup suffix=\".\"\x3e\n	\x3cgroup delimiter=\". \"\x3e\n	  \x3ctext macro=\"author\"/\x3e\n	  \x3ctext macro=\"issued\"/\x3e\n	\x3c/group\x3e\n        \x3cgroup delimiter=\". \"\x3e\n          \x3ctext macro=\"title\" prefix=\" \"/\x3e\n          \x3cgroup\x3e\n            \x3ctext macro=\"container-contributors\"/\x3e\n            \x3ctext macro=\"secondary-contributors\"/\x3e\n            \x3cgroup delimiter=\", \"\x3e\n	      \x3ctext macro=\"container\"/\x3e\n              \x3ctext variable=\"collection-title\"/\x3e\n            \x3c/group\x3e\n          \x3c/group\x3e\n        \x3c/group\x3e\n        \x3ctext macro=\"locators\"/\x3e\n        \x3cgroup delimiter=\", \" prefix=\". \"\x3e\n          \x3ctext macro=\"event\"/\x3e\n          \x3ctext macro=\"publisher\"/\x3e\n        \x3c/group\x3e\n      \x3c/group\x3e\n      \x3ctext macro=\"access\" prefix=\" \"/\x3e\n    \x3c/layout\x3e\n  \x3c/bibliography\x3e\n\x3c/style\x3e\n" }, "item": { "4fa907c366bf5": { "title": "Acadia Library: a wealth of resources", "container-title": "Valley Times", "genre": "Newspaper Article", "note": "1. Special supplement on Education Week.  2. Source type: Print(0).  ", "page": "6EW", "author": [ { "given": "Betty M.", "family": "Jeffery", "parse-names": "true" } ], "issued": { "raw": "1989" }, "id": "4fa907c366bf5" } }, "bibliography": { "4fa907c360668": { "id": "4fa907c360668", "style": "4fa907c35f6c9", "abbreviation": null, "items": { "4fa907c3677ab": { "citationItems": [ { "id": "4fa907c366bf5" } ], "properties": { "noteIndex": 0 } } } } } } });
//--><!]]>
</script>

</head>
<body>
HTML;
  
  foreach ($selections as $pid) {
    $mods = islandora_bibilgraphy_form_get_mods($pid);
    if ($mods) {
      $citation = citeproc_citation_from_mods($mods);
      $bibliography->addCitation($citation);
    }
  }
  if (isset($selections[0])) {
    $output .= $bibliography->render();
  }
  else {
    drupal_set_message(t('No citations were selected!'), 'warning');
  }
  $output .= l(t('Return to the Bibliography.'), 'bibliography');
  $output .= '</body>';
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