<?php

/**
 * @file
 */

/**
 * Cron function to remove the policy datastream from an object if the 
 * datastream has the label 'POLICY' and the mods.embargo_date
 * index field has expired in the last week. If the cron job is set up
 * properly then we sould only ever need to check the last week. Checking
 * further back would increase the number of objects that would need
 * to be checked.
 * Added extra check for a supplemental information embargo.
 */
function scholar_cron_embargo_check() {
  module_load_include('inc', 'fedora_repository', 'api/fedora_item');

  $path = drupal_get_path('module', 'scholar');

  $solr_location = variable_get('islandora_solr_search_block_url', 'localhost:8080/solr');

  $document_solr_query = 'http://' . $solr_location . '/select/?q=mods.embargo_date:[NOW/DAY-7DAY%20TO%20NOW/DAY]%20AND%20rels.hasModel:thesisCModel&fl=PID&rows=10000';
  $document_solr_result = @file_get_contents($document_solr_query);

  if ($document_solr_result != NULL) {
    $xml = new SimpleXMLElement($document_solr_result);
    $array = $xml->xpath('//doc');
    $array = array_filter($array);
    $pids = array();
    foreach ($array as $arr) {
      $pid = (string) $arr[0]->str;
      drupal_set_message(t('PID: @pid', array('@pid' => $pid)));
      $pids[] = $pid;
      $item = new Fedora_Item($pid);
      $ds_array = $item->get_datastreams_list_as_array();
      foreach ($ds_array as $key => $ds) {
        if ($key == 'POLICY') {
          $policy_details = $item->get_datastream_info('POLICY');
          drupal_set_message(t('Policy label: @label', array('@label' => $policy_details->datastream->label)));
          switch ($policy_details->datastream->label) {
            case 'POLICY':
            case 'Embargo policy - Both':
              $result_remove = $item->purge_datastream('POLICY');
              $result_add = $item->add_datastream_from_file($path . '/xacml/supplemental-embargo.xml', 'POLICY', 'Embargo policy - Supp');
              break;
            case 'Embargo policy - Doc';
              $result_remove = $item->purge_datastream('POLICY');
              break;
            case 'Embargo policy - Supp':
              drupal_set_message(t('Document embargo for object \'@pid\' already removed!', array('@pid' => $pid)), 'warning');
              break;
            default:
              drupal_set_message(t('Unknown policy on object \'@pid\', unable to remove!', array('@pid' => $pid)), 'warning');
          }
        }
      }
    }
  }

  $supp_solr_query = 'http://' . $solr_location . '/select/?q=mods.supp_embargo_date:[NOW/DAY-7DAY%20TO%20NOW/DAY]%20AND%20rels.hasModel:thesisCModel&fl=PID&rows=10000';
  $supp_solr_result = @file_get_contents($supp_solr_query);

  if ($supp_solr_result != NULL) {
    $xml = new SimpleXMLElement($supp_solr_result);
    $array = $xml->xpath('//doc');
    $array = array_filter($array);
    $pids = array();
    foreach ($array as $arr) {
      $pid = (string) $arr[0]->str;
      drupal_set_message(t('PID: @pid', array('@pid' => $pid)));
      $pids[] = $pid;
      $item = new Fedora_Item($pid);
      $ds_array = $item->get_datastreams_list_as_array();
      foreach ($ds_array as $key => $ds) {
        if ($key == 'POLICY') {
          $policy_details = $item->get_datastream_info('POLICY');
          drupal_set_message(t('Policy label: @label', array('@label' => $policy_details->datastream->label)));
          switch ($policy_details->datastream->label) {
            case 'POLICY':
            case 'Embargo policy - Both':
              $result_remove = $item->purge_datastream('POLICY');
              $result_add = $item->add_datastream_from_file($path . '/xacml/supplemental-embargo.xml', 'POLICY', 'Embargo policy - Doc');
              break;
            case 'Embargo policy - Supp';
              $result_remove = $item->purge_datastream('POLICY');
              break;
            case 'Embargo policy - Doc':
              drupal_set_message(t('Supplemental info embargo for object \'@pid\' already removed!', array('@pid' => $pid)), 'warning');
              break;
            default:
              drupal_set_message(t('Unknown policy on object \'@pid\', unable to remove!', array('@pid' => $pid)), 'warning');
          }
        }
      }
    }
  }

  // Remove POLICY datastream from objects that don't have an embargo set for either the document or
  // the supplemental information.

  $none_solr_query = 'http://' . $solr_location . '/select/?q=-mods.embargo_date:[*%20TO%20*]%20OR%20-mods.supp_embargo_date:[*%20TO%20*]%20AND%20rels.hasModel:thesis&fl=PID&rows=10000';
  $none_solr_result = @file_get_contents($none_solr_query);

  if ($none_solr_result != NULL) {
    $xml = new SimpleXMLElement($none_solr_result);
    $array = $xml->xpath('//doc');
    $array = array_filter($array);
    $pids = array();
    foreach ($array as $arr) {
      $pid = (string) $arr[0]->str;
      drupal_set_message(t('PID: @pid', array('@pid' => $pid)));
      $pids[] = $pid;
      $item = new Fedora_Item($pid);
      $ds_array = $item->get_datastreams_list_as_array();
      foreach ($ds_array as $key => $ds) {
        if ($key == 'POLICY') {
          $result_remove = $item->purge_datastream('POLICY');
          if ($result_remove != NULL) {
            drupal_set_message(t('Embargo policy for object @pid removed!', array('@pid' => $pid)));
          }
          else {
            drupal_set_message(t('Unable to remove embargo policy for object @pid!', array('@pid' => $pid)));
          }
        }
      }
    }
  }
}

function scholar_cron_citations_update() {
  $solr_location = variable_get('islandora_solr_search_block_url', 'localhost:8080/solr');
  $result = file_get_contents('http://' . $solr_location . '/select/?q=rels.hasModel:researcherCModel&fl=dc.identifier&rows=10000');
  $xml = new SimpleXMLElement($result);
  $array = $xml->xpath('//doc');
  $array = array_filter($array);
  foreach ($array as $arr) {
    $username = $arr->xpath('./arr[@name="dc.identifier"]/str');
    if ($username[0] != '') {
      $username_array = explode(':', (string) $username[0]);
      $username = strtolower($username_array[1]);
      $username_str = 'http://' . $solr_location . '/select/?q=mods.username:' . (str_replace(array(';', '_'), array(' ', '%20'), $username));
      $username_query = file_get_contents($username_str);
      $username_xml = new SimpleXMLElement($username_query);
      $number_of_citations = $username_xml->xpath('./result[@name="response"]/@numFound');
      $citations = (string) $number_of_citations[0];
      $query = db_fetch_array(db_query("SELECT id FROM {islandora_scholar_citation_counts} WHERE pid = '%s'", $username));
      if ($query) {
        $result = db_query("UPDATE {islandora_scholar_citation_counts} SET citation_count = '%d' WHERE pid = '%s'", $citations, $username);
      }
      else {
        $result = db_query("INSERT INTO {islandora_scholar_citation_counts} (pid,citation_count) VALUES ('%s', '%d')", $username, $citations);
      }
    }
  }

  $result_dept = file_get_contents('http://' . $solr_location . '/select/?q=rels.hasModel:departmentCModel&fl=mads.identifier_sort&rows=10000');
  $xml_dept = new SimpleXMLElement($result_dept);
  $array_dept = $xml_dept->xpath('//doc');
  var_dump($array_dept);
  $array_dept = array_filter($array_dept);
  foreach ($array_dept as $arr_dept) {
    drupal_set_message('Dept: ' . $arr_dept);    
    $dept = $arr_dept->xpath('./arr[@name="mads.identifier_sort"]/str');
    if ($dept[0] != '') {
      $dept_copy = str_replace('_', ' ', $dept[0]);
      $dept = strtolower($dept[0]);

      if ($dept_copy == 'Pathology and Microbiology') {
        $dept_copy = 'Path Micro" OR mods.department_facet~~"Pathology and Microbiology';
      }

      if ($dept_copy == 'School of Business') {
        $dept_copy = 'School of Business" OR mods.department_facet~~"Business';
      }

      if ($dept_copy == 'Political Science') {
        $dept_copy = 'Political Studies" OR mods.department_facet~~"Political Science';
      }

      if ($dept_copy == 'Sociology and Anthropology') {
        $dept_copy = 'Sociology and Anthropology" OR mods.department_facet~~"Soc Anth';
      }

      if ($dept_copy == 'Women\'s Studies') {
        $dept_copy = 'Women\'s Studies" OR mods.department_facet~~"Womens Studies';
      }

      if ($dept_copy == 'Department of Biomedical Sciences') {
        $dept_copy = 'Biomedical Sciences" OR mods.department_facet~~"Department of Biomedical Sciences';
      }

      if ($dept_copy == 'Applied Human Sciences') {
        $dept_copy = 'Applied Human Sciences" OR mods.department_facet~~"Home Economics';
      }

      if ($dept_copy == 'Biomedical Sciences') {
        $dept_copy = 'Biomedical Sciences" OR mods.department_facet~~"Anatomy Physiology';
      }

      if ($dept_copy == 'Computer Science and Information Technology') {
        $dept_copy = 'Computer Science and Information Technology" OR mods.department_facet~~"Computer Science and Info Technology';
      }

      $dept_str = 'http://' . $solr_location . '/select/?q=mods.department_facet:"' . (str_replace(array(';', '_', ' ', '&', '~~'), array(' ', '%20', '%20', '?', ':'), $dept_copy)) . '"';
      $dept_query = file_get_contents($dept_str);
      $dept_xml = new SimpleXMLElement($dept_query);
      $number_of_citations = $dept_xml->xpath('./result[@name="response"]/@numFound');
      $citations = (string) $number_of_citations[0];
      $query = db_fetch_array(db_query("SELECT id FROM {islandora_scholar_citation_counts} WHERE pid = '%s'", $dept));
      if ($query) {
        $result = db_query("UPDATE {islandora_scholar_citation_counts} SET citation_count = '%d' WHERE pid = '%s'", $citations, $dept);
      }
      else {
        $result = db_query("INSERT INTO {islandora_scholar_citation_counts} (pid,citation_count) VALUES ('%s', '%d')", $dept, $citations);
      }
    }
  }
}