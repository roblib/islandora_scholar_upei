<?php

/**
 * @file
 *
 * Implements a class for interacting with the stored CSL files.
 */
class CSL {
  /**
   * The name of the table where the CSL styles are stored.
   */
  const table = 'csl';

  /**
   * Checks to see if the given CSL style is a valid definition.
   *
   * @param DOMDocument $style
   *   A CSL 1.0 style file to check to see if it is valid.
   *
   * @return boolean
   *   TRUE if the given style is valid, FALSE otherwise.
   */
  public static function Valid(DOMDocument $style) {
    $schema_filename = drupal_get_path('module', 'csl') . '/schema/csl.rng';
    return $style->relaxNGValidate($schema_filename);
  }

  /**
   * Checks to see if the given CSL style exists in the database.
   *
   * @param string $style_name
   *   The name of the CSL style.
   *
   * @return boolean
   *   TRUE if the given style exists, FALSE otherwise.
   */
  public static function Exists($style_name) {
    $query = db_query('SELECT count(name) FROM {%s} WHERE name = "%s"', self::table, $style_name);
    $count = db_result($query);
    return $count == 1;
  }

  /**
   * Gets a list of all the availible styles.
   *
   * @return array
   *   An array of defined style's names, where both the key and the value are the style's name.
   *   EX. array( 'name' => 'name' );
   */
  public static function GetNames() {
    $names = array();
    $result = db_query('SELECT name FROM {%s}', self::table);
    while ($data = db_fetch_object($result)) {
      $names[$data->name] = $data->name;
    }
    return $names;
  }

  /**
   * Gets the CSL style definition identified by name.
   *
   * @param string $style_name
   *   The name of the CSL style.
   *
   * @return DOMDocument
   *   The CSL style definition if found, FALSE otherwise.
   */
  public static function Get($style_name) {
    if (self::Exists($style_name)) {
      $query = db_query('SELECT style FROM {%s} WHERE name = "%s"', self::table, $style_name);
      $xml = db_result($query);
      if (trim($xml) !== '') {
        $definition = new DOMDocument();
        $definition->loadXML($xml);
        return $definition;
      }
    }
    return FALSE;
  }

  /**
   * Creates a CSL style with the given style name and definition.
   *
   * If the CSL style already exists it will fail.
   *
   * @param string $style_name
   *   The name of the CSL style.
   * @param DOMDocument $definition
   *   A CSL style definition.
   *
   * @return boolean
   *  TRUE if successful, otherwise FALSE.
   */
  public static function Create($style_name, DOMDocument $definition) {
    if (!self::Exists($style_name)) {
      if (!self::Valid($definition)) {
        drupal_set_message("The $style_name is not valid CSL 1.0 problems may occur.");
      }
      $object = new stdClass();
      $object->name = $style_name;
      $object->style = $definition->saveXML();
      return drupal_write_record(self::table, $object) !== FALSE;
    }
    return FALSE;
  }

  /**
   * Deletes the style with the given name.
   *
   * @param string $style_name
   *   The name of the CSL style definition.
   *
   * @return boolean
   *   TRUE if successful FALSE otherwise.
   */
  public static function Delete($style_name) {
    if (self::Exists($style_name)) {
      db_query('DELETE FROM {%s} WHERE name = "%s"', self::table, $style_name);
      return TRUE;
    }
    return FALSE;
  }

}