<?php

// $Id$

/**
 * @file
 *
 */
module_load_include('inc', 'fedora_repository', 'api/fedora_item');
module_load_include('inc', 'content_model_viewer', 'models/AbstractModel');

/**
 * Models the display information a field books should have.
 */
class CitationModel extends AbstractModel {

  /**
   * Object Pid.
   * 
   * @var string
   */
  protected $pid;

  /**
   * The Object.
   * 
   * @var Fedora_Item
   */
  protected $item;

  /**
   * Creates CitationModel instance.
   * 
   * @param string $pid 
   */
  public function __construct($pid) {
    $this->pid = $pid;
    $this->item = new Fedora_Item($pid);
  }

  /**
   * The function required for this viewer to render correctly.
   */
  public function getInitializationFunction() {
    return NULL; // Nothing at the moment...
  }

  /**
   * Adds all of the required javascript/css/etc that this model needs to be shown.
   */
  public function addRequiredResources() {
    module_load_include('inc', 'citeproc', 'CiteProcJS');
    drupal_add_css(drupal_get_path('module', 'scholar') . '/css/Record.css');
    CiteProcJS::addResources(); // Includes javascript and other fun things.
  }

  /**
   * Gets HTML to be rendered into the overview Panel.
   * 
   * @return string
   */
  public function render() {
    return theme(THEME_SCHOLAR_OVERVIEW_PANEL_CITATION, $this->pid);
  }

  /**
   * Gets the HTML required for the Citation.
   * 
   * @return string
   */
  public function getCitation($style_name) {
    if (isset($this->item->datastreams['MODS'])) {
      $mods = $this->item->get_datastream_dissemination('MODS');
      $mods = trim($mods);
      if (!empty($mods)) {
        $doc = new DOMDocument();
        $doc->loadXML($mods);
        $style = citeproc_style($style_name);
        $bibliography = citeproc_bibliography_from_mods($style, $doc);
        return array($bibliography->getID(), $bibliography->render());
      }
    }
    return array(NULL, '');
  }

}

/**
 * Preprocess some junk for the overview.
 * 
 * @param array $variables 
 */
function template_preprocess_scholar_overview_panel_citation(array &$variables) {
  module_load_include('inc', 'csl', 'CSL');
  module_load_include('inc', 'islandora_bibliography', 'CitationForm');
  $pid = $variables['pid'];
  $item = new Fedora_Item($pid);
  $model = new CitationModel($pid);
  // Citations
  $styles = CSL::GetNames();
  foreach ($styles as $name) {
    list($id, $html) = $model->getCitation($name);
    $variables['citation'][] = $html;
    $variables['styles'][$id] = $name;
  }
  // Abstract
  if (isset($item->datastreams['MODS'])) {
    $mods = $item->get_datastream_dissemination('MODS');
    $mods = trim($mods);
    if (!empty($mods)) {
      $mods = new SimpleXMLElement($mods);
      $mods->registerXPathNamespace('mods', 'http://www.loc.gov/mods/v3');
      $variables['abstract'] = implode('<br/>', $mods->xpath('/mods:mods/mods:abstract'));
      $variables['subject'] = implode('<br/>', $mods->xpath('/mods:mods/mods:subject/mods:topic'));
      $variables['grant_number'] = implode('<br/>', $mods->xpath('/mods:mods/mods:name[@type="corporate"]/mods:role[mods:roleTerm[@type="text"]="funding agency"]/../mods:description[@type="grant number"]'));
      $variables['funding_agency'] = implode('<br/>', $mods->xpath('/mods:mods/mods:name[@type="corporate"]/mods:role[mods:roleTerm[@type="text"]="funding agency"]/../mods:namePart'));
      $variables['core_facility'] = implode('<br/>', $mods->xpath('/mods:mods/mods:note[@type="core facilities"]'));
      $variables['source_system'] = implode('<br/>', $mods->xpath('/mods:mods/mods:recordInfo/mods:recordOrigin'));
      $variables['ccsg_program'] = implode('<br/>', array()); // @todo
      $variables['peer_reviewed'] = implode('<br/>', $mods->xpath('/mods:mods/mods:note[@type="peer reviewed"]'));
    }
  }
}