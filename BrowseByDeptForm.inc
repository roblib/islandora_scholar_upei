<?php

/**
 * @file
 * 
 */

function scholar_browse_by_dept_form() {
  module_load_include('inc', 'Fedora_Repository', 'api/fedora_utils');
  module_load_include('inc', 'islandora_solr_search', 'IslandoraSolrQueryProcessor');
  
  $result = file_get_contents('http://localhost:8080/solr/select/?q=rels.hasModel:departmentCModel&fl=mads.identifier&rows=500&sort=mads.identifier_sort+asc');
    
  $xml = new SimpleXMLElement($result);
  
  $depts = array_unique($xml->xpath('//doc/arr/str'));
  $output = '<ul>';
  foreach ($depts as $dept) {
    $dept_copy = $dept;
    if ($dept_copy == 'Pathology and Microbiology') {
      $dept_copy = 'Path Micro';
    }
    if ($dept_copy == 'Political Science') {
      $dept_copy = 'Political Studies';
    }
    $dept_result = file_get_contents('http://localhost:8080/solr/select/?q=mods.department:"' . (str_replace(array(' ', ":", '&'), array('%20', '?', '?'), ($dept_copy))) . '"');
    $dept_xml = new SimpleXMLElement($dept_result);
    $number_of_citations = $dept_xml->xpath('./result[@name="response"]/@numFound');
    $citations = " (" . (string) $number_of_citations[0] . ")";
    $dept_link = str_replace(' ', '%20', $dept_copy);
    $output .= '<li><a href=islandora/solr/search/mods.department:"' . $dept_link . '">' . $dept . '</a><i>' . $citations . '</i></li>';
  }
//  $output = $result;
//  $result = db_query('SELECT role.rid, role.name FROM {role} where role.rid > %d order by role.name', 0);
//  $output .= '<ul>';
//  while ($role = db_fetch_object($result)) {
//
//    if ($role->rid > $ignore_roles_less_than && in_array($role->name, $inclusionlist)) {
//      $urlportion = urlencode($role->name);
//      $name = $role->name;
//      if ('Home Economics' == $name) {
//        $name = "Family and Nutritional Sciences";
//      }
//      else
//      if ('Anatomy Physiology' == $name) {
//        $name = "Biomedical Sciences";
//      }
//      else
//      if ('Womens Studies' == $name) {
//        $name = "Women's Studies";
//      }
//      else
//      if ('Path Micro' == $name) {
//        $name = "Pathology and Microbiology";
//      }
//      else
//      if ('Soc Anth' == $name) {
//        $name = "Sociology and Anthropology";
//      }
//      $output .= "<li><a href=islandora/solr/search/$urlportion>" . $name . '</a></li>';
//    }
//  }
  $output .= '</ul>';

//  $form['list_ahah_wrapper'] = array(
//    '#prefix' => '<div id="dept-list">',
//    '#suffix' => '</div>',
//  );
//  
//  $form['list_ahah_wrapper']['islandora_book_do_ocr'] = array(
//    '#type' => 'checkbox',
//    '#title' => t('Perform OCR on incoming TIFF images?'),
//    '#description' => t('Do not check this box if OCR is performed on an exernal server, such as ABBYY'),
//    '#default_value' => TRUE,
//    '#ahah' => array(
//      'path' => 'islandora/dept_list',
//      'wrapper' => 'dept-list',
//      'effect' => 'fade',
//      'event' => 'change'),
//  );
  
  $form['list'] = array(
    '#value' => $output,
  );
  
  return $form;
}

/**
 * update ocr div ??
 */
function update_dept_list_div() {

  $form = dept_list_callback_prep();
  $changed_elements = $form['dept_list_ahah_wrapper'];
  unset($changed_elements['#prefix'], $changed_elements['#suffix']);
  $output = theme('status_messages') . drupal_render($changed_elements);
  drupal_json(array(
    'status' => TRUE,
    'data' => $output,
  ));
}

/**
 * book callback prep??
 * @return type 
 */
function dept_list_callback_prep() {
  $form_state = array('storage' => NULL, 'submitted' => FALSE);
  $form_build_id = $_POST['form_build_id'];
  $form = form_get_cache($form_build_id, $form_state);
  $args = $form['#parameters'];
  $form_id = array_shift($args);
  $form_state['post'] = $form['#post'] = $_POST;
  // Enable the submit/validate handlers to determine whether AHAH-submittted.
  $form_state['ahah_submission'] = TRUE;
  $form['#programmed'] = $form['#redirect'] = FALSE;
  drupal_process_form($form_id, $form, $form_state);
  $form = drupal_rebuild_form($form_id, $form_state, $args, $form_build_id);
  return $form;
}