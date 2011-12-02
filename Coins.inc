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
  
  /**
   * Create the object properties from the MODS datastream
   * @param type $pid 
   * @return type 
   */
  function __construct($pid) {
    $this->pid = $pid;
    module_load_include('inc', 'Fedora_Repository', 'CollectionClass');
    $collectionHelper = new CollectionClass();
    $xmlstr = $collectionHelper->getStream($this->pid, "MODS", 0);

    if ($xmlstr == null || strlen($xmlstr) < 5) {
      drupal_set_message(t('No MODS datastream found!', 'warning'));
      return " ";
    }
    $xml = new SimpleXMLElement($xmlstr);
    $xml->registerXPathNamespace('mods', 'http://www.loc.gov/mods/v3');
    
    $this->mods = $xml;
    
    $issn = $xml->xpath('//mods:identifier[@type="issn"]');
    $this->issn = str_replace(array("&", "=", ",", ";"), array('', '', '', ''), (string) $issn[0]);
    
    $this->date = $xml->xpath('//mods:part/mods:date');
    
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
  }
  
}