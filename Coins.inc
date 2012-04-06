<?php

/**
 * @file
 * Coins.inc
 */

/**
 * Class to parse relevant data from MODS to create the COINS URL
 */
class Coins {

  public $pid = NULL;
  public $mods = NULL;
  public $issn = NULL;
  public $volume = NULL;
  public $issue = NULL;
  public $date = NULL;
  public $start_page = NULL;
  public $title = NULL;
  public $sub_title = NULL;
  public $journal_title = NULL;
  public $abstract = NULL;
  public $names = NULL;
  public $authors = NULL;
  public $reverse_names = NULL;
  public $topic = NULL;
  public $note = NULL;
  public $embargo_date = NULL;

  /**
   * Create the object properties from the MODS datastream
   * @param type $pid 
   * The PID of the object that you want to extract the MODS data from
   * @return type 
   */
  function __construct($pid) {
    $this->pid = $pid;
    module_load_include('inc', 'Fedora_Repository', 'CollectionClass');
    $collectionHelper = new CollectionClass();
    $xmlstr = $collectionHelper->getStream($this->pid, "MODS", 0);

    if ($xmlstr == NULL || strlen($xmlstr) < 5) {
      drupal_set_message(t('No MODS datastream found!', 'warning'));
      return " ";
    }
    $xml = new SimpleXMLElement($xmlstr);
    $xml->registerXPathNamespace('mods', 'http://www.loc.gov/mods/v3');

    $this->mods = $xml;

    $full_name = array();
    $authors = array();
    $i = 0;
    foreach ($xml->name as $name) {
      foreach ($name->namePart as $name_part) {
        if ($name_part['type'] == 'given') {
          $full_name[$i] .= (string) substr(rtrim($name_part, '.'), 0, 1) . '. ';
        }
        if ($name_part['type'] == 'family') {
          $full_name[$i] .= (string) rtrim($name_part, '.');
        }
      }
      $role = (string) $name->role->roleTerm;
      if ($role == 'aut') {
        $role = 'author';
      }
      var_dump($role);
      var_dump($full_name);
      if ($role == 'author') {
        $authors[] = $fullname;
      }
      $full_name[$i] .= ' (' . $role . ')';
      $i++;
    }
    $this->names = $full_name;
    $this->authors = $authors;
    
    $reverse = array();
    $i = 0;
    foreach ($xml->name as $name) {
      foreach ($name->namePart as $name_part) {
        if ($name_part['type'] == 'family') {
          $reverse_family[$i] .= (string) rtrim($name_part, '.');
        }
        if ($name_part['type'] == 'given') {
          $reverse_given[$i] .= (string) rtrim($name_part, '.');
        }
        $reverse[$i] = $reverse_family[$i] . ', ' . $reverse_given[$i] . '.';
      }
      $i++;
    }
    $this->reverse_names = $reverse;

    $issn = $xml->xpath('//mods:identifier[@type="issn"]');
    $this->issn = str_replace(array("&", "=", ",", ";"), array('', '', '', ''), (string) $issn[0]);

    $this->date = $xml->part->date;

    $volume = $xml->xpath('//mods:part/mods:detail[@type="volume"]');
    $this->volume = $volume[0]->number;

    $issue = $xml->xpath('//mods:part/mods:detail[@type="issue"]');
    $this->issue = $issue[0]->number;

    $start_page = $xml->xpath('//mods:extent[@unit="page"]/mods:start');
    $this->start_page = $start_page[0];

    $title = $xml->xpath('//mods:title');
    $this->title = $title[0];

    $sub_title = $xml->xpath('//mods:title');
    $this->sub_title = $sub_title[0];

    $journal_title = $xml->xpath('//mods:relatedItem[@type="host"]//mods:title');
    $this->journal_title = $journal_title[0];

    $abstract = $xml->xpath('//mods:abstract');
    $this->abstract = $abstract[0];
    
    $topic = array();
    foreach ($xml->subject as $subject) {
      $topic[] = $subject->topic;
    }
    $this->topic = $topic;
    
    $notes = array();
    foreach ($xml->note as $note) {
      $notes[] = $note;
    }
    $this->note = $note;
    
    $embargo_date = $xml->xpath('//mods:dateOther[@type="embargo"]');
    $this->embargo_date = $embargo_date[0];
  }

}