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
  
  $result_dept = file_get_contents('http://' . $solr_location . '/select/?q=rels.hasModel:departmentCModel&fl=dc.identifier&rows=10000');
  $xml_dept = new SimpleXMLElement($result_dept);
  $array_dept = $xml_dept->xpath('//doc');
  $array_dept = array_filter($array_dept);
  foreach ($array_dept as $arr_dept) {
    $dept = $arr_dept->xpath('./arr[@name="dc.identifier"]/str');
    if ($dept[0] != '') {
      $dept_array = explode(':', (string) $dept[0]);
      $dept = strtolower($dept_array[1]);      
      $dept_copy = $dept;
      
    if ($dept_copy == 'pathology and microbiology') {
      $dept_copy = 'path micro" OR mods.department_facet~~"pathology and microbiology';
    }

    if ($dept_copy == 'school of business') {
      $dept_copy = 'school of business" OR mods.department_facet~~"business';
    }

    if ($dept_copy == 'political science') {
      $dept_copy = 'political studies" OR mods.department_facet~~"political science';
    }

    if ($dept_copy == 'sociology and anthropology') {
      $dept_copy = 'sociology and anthropology" OR mods.department_facet~~"soc anth';
    }

    if ($dept_copy == 'women\'s studies') {
      $dept_copy = 'women\'s studies" OR mods.department_facet~~"womens studies';
    }

    if ($dept_copy == 'department of biomedical sciences') {
      $dept_copy = 'biomedical sciences" OR mods.department_facet~~"department of biomedical sciences';
    }

    if ($dept_copy == 'applied human sciences') {
      $dept_copy = 'applied human sciences" OR mods.department_facet~~"home economics';
    }

    if ($dept_copy == 'biomedical sciences') {
      $dept_copy = 'biomedical sciences" OR mods.department_facet~~"anatomy physiology';
    }

    if ($dept_copy == 'computer science and information technology') {
      $dept_copy = 'computer science and information technology" OR mods.department_facet~~"computer science and info technology';
    }      
      
      
      $dept_str = 'http://' . $solr_location . '/select/?q=mods.department:' . (str_replace(array(';', '_'), array(' ', '%20'), $dept_copy));
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