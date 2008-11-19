<?php
// $Id$

/**
 * @file
 * Common functions for Location tests.
 */

class LocationTestCase extends DrupalWebTestCase {

  /**
   * Flatten a post settings array because drupalPost isn't smart enough to.
   */
  function flattenPostData(&$edit) {
    do {
      $edit_flattened = TRUE;
      foreach ($edit as $k => $v) {
        if (is_array($v)) {
          $edit_flattened = FALSE;
          foreach ($v as $kk => $vv) {
            $edit["{$k}[{$kk}]"] = $vv;
          }
          unset($edit[$k]);
        }
      }
    } while (!$edit_flattened);
  }

  function addLocationContentType(&$settings, $add = array()) {
    // find a non-existent random type name.
    do {
      $name = strtolower($this->randomName(3, 'type_'));
    } while (node_get_types('type', $name));

    // Get the (settable) defaults.
    $defaults = array();
    $d = location_invoke_locationapi($location, 'defaults');
    $fields = location_field_names();
    foreach ($fields as $k => $v) {
      $defaults[$k] = $d[$k];
    }

    foreach ($defaults as $k => $v) {
      // Change collection to allow.
      $defaults[$k]['collect'] = 1;
    }

    $settings = array(
      'name' => $name,
      'type' => $name,
      'location_settings' => array(
        'multiple' => array(
          'max' => 1,
        ),
        'form' => array(
          'fields' => $defaults,
        ),
      ),
    );

    //$settings['location_settings'] = array_merge_recursive($settings['location_settings'], $add);
    $this->flattenPostData($settings);
    $add = array('location_settings' => $add);
    $this->flattenPostData($add);
    $settings = array_merge($settings, $add);
    $this->drupalPost('admin/content/types/add', $settings, 'Save content type');
    $this->refreshVariables();
    $settings = variable_get('location_settings_node_'. $name, array());
    return $name;
  }

}
