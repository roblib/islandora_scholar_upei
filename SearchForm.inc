<?php

/**
 * @file
 *
 * Contains the hooks for rendering and processing the Quick Search form.
 */

/**
 * Builds a drupal form for launching a quick search.
 * 
 * @param array $form_state 
 * 
 * @return array
 */
function scholar_search_form(array &$form_state) {
  drupal_add_js(drupal_get_path('module', 'scholar') . '/js/Ahah.js');
  drupal_add_css(drupal_get_path('module', 'scholar') . '/css/Search.css');
  $num_terms = 1;
  if (isset($form_state['values'])) {
    $values = $form_state['values'];
    $num_terms = isset($form_state['num_terms']) ? $form_state['num_terms'] : $num_terms;
  }
  elseif (isset($_SESSION['scholar']['search']['values'])) {
    /**
     * Handles the changing of URL from the initial search page to the actual search page.
     */
    $values = $_SESSION['scholar']['search']['values'];
    $num_terms = count($values['terms']);
    unset($_SESSION['scholar']['search']['values']); // Forget this for the next page visit.
  }
  $terms = array(
    '#prefix' => '<div id="scholar-search-terms">',
    '#suffix' => '</div>',
    '#tree' => TRUE,
  );
  for ($i = 0; $i < $num_terms; $i++) {
    $term = array(
      '#tree' => TRUE,
      '#prefix' => '<div>',
      '#suffix' => '</div>',
      'field' => array(
        '#title' => t('Field'),
        '#type' => 'select',
        '#default_value' => isset($values['terms'][$i]['field']) ? $values['terms'][$i]['field'] : 'mods.title',
        '#options' => array(
          'mods.abstract' => t('Abstract'),
          'mods.roleTerm' => t('Author Identifier'),
          'mods.Author' => t('Author'),
          'mods.title' => t('Title'),
          'mods.isbn' => t('ISBN'),
          'mods.issn' => t('ISSN'),
          'mods.hostTitle' => t('Full Journal Title'),
          'mods.type' => t('Publication Type'),
          'dc.title' => t('Title'),
          'mods.doi' => t('DOI'),
          'mods.subject' => t('Keywords'),
          'mods.date' => t('Year published'),
        )
      ),
      'search' => array(
        '#title' => t('Search'),
        '#type' => 'textfield',
        '#default_value' => isset($values['terms'][$i]['search']) ? $values['terms'][$i]['search'] : NULL,
      ),
      'hidden_submit' => array(// Used for when the user presses enter on the search field.
        '#type' => 'submit',
        '#value' => 'Search',
        '#attributes' => array('style' => 'visibility:hidden;position:fixed;top:-1000px;right:-1000px;')
      ),
      'add' => array(
        '#type' => 'button',
        '#value' => 'Add Field',
        '#ahah' => array(
          'path' => MENU_SCHOLAR_ADVANCED_SEARCH_ADD_FIELD,
          'wrapper' => 'scholar-search-terms',
          'method' => 'replace',
          'effect' => 'fade'
        )
      ),
    );
    if ($i > 0) {
      $term['remove'] = array(
        '#type' => 'button',
        '#field' => $i,
        '#value' => 'Remove Field',
        '#name' => 'remove-field-' . $i,
        '#ahah' => array(
          'path' => MENU_SCHOLAR_ADVANCED_SEARCH_REMOVE_FIELD,
          'wrapper' => 'scholar-search-terms',
          'method' => 'replace',
          'effect' => 'fade'
        )
      );
    }
    $terms[] = $term;
  }

  $researchers = array();
  $result = file_get_contents('http://localhost:8080/solr/select/?q=rels.hasModel:researcherCModel&fl=mads.family,mads.given&rows=10000');
  $xml = new SimpleXMLElement($result);
  $array = $xml->xpath('//doc');
  foreach ($array as $arr) {
    $family = $arr->xpath('./arr[@name="mads.family"]/str');
    $given = $arr->xpath('./arr[@name="mads.given"]/str');
    $researchers[(string) $given[0] . ' ' . (string) $family[0]] = (string) $family[0] . ', ' . (string) $given[0];
    }

  asort($researchers);
  
  return array(
    'terms' => $terms,
    'controls' => array(
      '#type' => 'markup',
      '#prefix' => '<div class="scholar-search-controls">',
      '#suffix' => '</div>',
      'sort' => array(
        '#title' => t('Sort By'),
        '#type' => 'select',
        '#default_value' => isset($values['sort']) ? $values['sort'] : NULL,
        '#options' => array(
          NULL => t('Relevance'),
          'mods.title' => t('Title'),
          'mods.date' => t('Date Added'),
        ),
      ),
      'order' => array(
        '#title' => t('Order'),
        '#type' => 'select',
        '#default_value' => isset($values['order']) ? $values['order'] : 'asc',
        '#options' => array(
          'asc' => t('Ascending'),
          'desc' => t('Descending'),
        ),
      ),
      'submit' => array(
        '#type' => 'submit',
        '#value' => 'Search',
        '#button' => 'search',
      )
    ),
  );
}

/**
 * Submit hook for the quick search form.
 * 
 * @param array $form
 * @param array $form_state 
 */
function scholar_search_form_submit(array $form, array &$form_state) {
  $button = $form_state['clicked_button']['#button'];
  $order = $form_state['values']['order'];
  if ($button == 'search') {
  $_SESSION['scholar']['search']['values'] = $form_state['values'];
  $_SESSION['scholar']['search']['sort'] = $form_state['values']['sort'];
  $_SESSION['scholar']['search']['order'] = $form_state['values']['order'];
  $query = scholar_search_form_build_query($form_state);
  $form_state['redirect'] = 'islandora/solr/search/' . urlencode($query) . '/'; // Redirect to the search.
  }
  else {
    $query = $form_state['values']['author'];
    $form_state['redirect'] = 'islandora/solr/search/mods.author:' . urlencode($query) . '/';
  }
}

/**
 *
 * @param array $form_state 
 */
function scholar_search_form_build_query(array &$form_state) {
  $query = 'dc.title:*';
  $statements = array();
  foreach ($form_state['values']['terms'] as $term) {
    $field = $term['field'];
    $search = trim($term['search']);
    if (!empty($search)) {
      $statements[] = "$field:$search";
    }
  }
  $query = !empty($statements) ? implode(' AND ', $statements) : '*'; // Empty return all results. 
  return $query;
}

/**
 *
 * @param array $form_state 
 */
function scholar_search_form_store_sort_in_session(array &$form_state) {
  $sort = $form_state['values']['sort'];
  unset($_SESSION['scholar']['search']);
  if (!empty($sort)) {
    $order = $form_state['values']['order'];
    $_SESSION['scholar']['search']['sort'] = $sort . ' ' . $order;
  }
}

function scholar_search_advanced_add_field() {
  module_load_include('inc', 'xml_form_elements', 'Ahah');
  list($form_id, $form_build_id, $form, $form_state) = Ahah::getFormInfo();
  $form = Ahah::processForm($form_id, $form, $form_state);
  $form_state['num_terms'] = count(element_children($form['terms'])) + 1;
  $form = Ahah::rebuildForm($form_id, $form_build_id, $form, $form_state);
  Ahah::respond($form['terms']);
}

function scholar_search_advanced_remove_field() {
  module_load_include('inc', 'xml_form_elements', 'Ahah');
  list($form_id, $form_build_id, $form, $form_state) = Ahah::getFormInfo();
  $form = Ahah::processForm($form_id, $form, $form_state);
  $field = $form_state['clicked_button']['#field'];
  array_splice($form_state['values']['terms'], $field, 1);
  $form_state['num_terms'] = count($form_state['values']['terms']);
  $form = Ahah::rebuildForm($form_id, $form_build_id, $form, $form_state);
  Ahah::respond($form['terms']);
}

function browse_by_department() {
  module_load_include('inc', 'Fedora_Repository', 'api/fedora_utils');
  module_load_include('inc', 'islandora_solr_search', 'IslandoraSolrQueryProcessor');
  $filename = drupal_get_path('module', 'scholar') . '/inclusionList.txt';
  $inclusionlist = file_get_contents($filename);
  $inclusionlist = fix_encoding($inclusionlist);
  $inclusionlist = explode(",", $inclusionlist);
  $ingnore_roles_less_than = 0;
  
  $result = file_get_contents('http://localhost:8080/solr/select/?q=rels.hasModel:researcherCModel&fl=mads.topic&rows=500');
  
  $xml = new SimpleXMLElement($result);
  
  $depts = array_unique($xml->xpath('//doc/arr/str'));
  $output = '<ul>';
  foreach ($depts as $dept) {
    if (array_search($dept, $inclusionlist)) {
    $output .= "<li><a href=islandora/solr/search/mads.topic:$dept>" . $dept . '</a></li>';
    }
  }
//  $output = $result;
//  $result = db_query('SELECT role.rid, role.name FROM {role} where role.rid > %d order by role.name', 0);
//  $output .= '<ul>';
//  while ($role = db_fetch_object($result)) {
//
//    if ($role->rid > $ingnore_roles_less_than && in_array($role->name, $inclusionlist)) {
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

  return $output;
}

function scholar_rss_feed($researcher) {
  
  $query_url = 'http://localhost:8080/solr/select?q=mods.username:' . $researcher . '&wt=xslt&tr=example_rss.xsl';
  $result = file_get_contents($query_url);
  drupal_set_header('Content-Type: text/xml; charset=utf-8');
  print $result;
}