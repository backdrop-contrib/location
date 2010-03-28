<?php
// $Id$

/**
 * @file
 * Coordinates field handler.
 */

class location_handler_field_location_distance extends views_handler_field {

  function option_definition() {
    $options = parent::option_definition();
    $options['origin'] = array('default' => 'user');
    $options['units'] = array('default' => 'km');
    $options['latitude'] = array('default' => '');
    $options['longitude'] = array('default' => '');
    return $options;
  }

  function has_extra_options() {
    return TRUE;
  }

  function extra_options_form(&$form, &$form_state) {
    $form['units'] = array(
      '#type' => 'radios',
      '#title' => t('Units'),
      '#options' => array(
        'km' => t('Kilometers'),
        'mi' => t('Miles'),
      ),
      '#description' => t('FIXME'),
      '#default_value' => $this->options['units'],
    );
    $form['origin'] = array(
      '#type' => 'radios',
      '#title' => t('Origin'),
      '#options' => array(
        'user' => t("User's location (blank if unset)"),
        'hybrid' => t("User's location (fall back to static if unset)"),
        'static' => t("Static location"),
        'tied' => t("Use Distance / Proximity filter"),
      ),
      '#description' => t('FIXME'),
      '#default_value' => $this->options['origin'],
    );
    $form['latitude'] = array(
      '#type' => 'textfield',
      '#title' => t('Latitude'),
      '#default_value' => $this->options['latitude'],
    );
    $form['longitude'] = array(
      '#type' => 'textfield',
      '#title' => t('Longitude'),
      '#default_value' => $this->options['longitude'],
    );
  }

  function click_sort($order) {
    $location = $this->_get_reference_location();

    if ($location) {
      $this->query->add_orderby(NULL, earth_distance_sql($location['longitude'], $location['latitude'], $this->table_alias), $order, $this->field_alias);
    }
  }

  function render($values) {
    if (empty($values->{$this->field_alias}) || $values->{$this->field_alias} == 'Unknown') {
      // Unset location, empty display.
      return;
    }
    $dist = (float)$values->{$this->field_alias};
    if ($this->options['units'] == 'km') {
      $dist = $dist / 1000.0;
      return theme('location_distance', $dist, 'km');
    }
    else {
      $dist = $dist / 1609.347;
      return theme('location_distance', $dist, 'mi');
    }
  }

  function query() {
    $this->ensure_my_table();

    $coordinates = $this->_get_reference_location();
    if ($coordinates) {
      $this->field_alias = $this->query->add_field(NULL, earth_distance_sql($coordinates['longitude'], $coordinates['latitude'], $this->table_alias), $this->table_alias .'_'. $this->field);
    }
    else {
      $this->field_alias = $this->query->add_field(NULL, "'Unknown'", $this->table_alias .'_'. $this->field);
    }
  }

  /**
   * Retrieves the coordinates of the location that this field measures
   * distances against.
   *
   * @return array with keys 'latitude' and 'longitude'; null if none was configured
   */
  function _get_reference_location() {
    if ($this->options['origin'] == 'hybrid' || $this->options['origin'] == 'user') {
      global $user;
      if (!isset($user->locations)) {
        $user = user_load($user->uid);
      }
      if (isset($user->locations[0]['latitude']) || !empty($user->locations[0]['latitude'])) {
        $latitude = (float)$user->locations[0]['latitude'];
        $longitude = (float)$user->locations[0]['longitude'];
      }
    }

    if ($this->options['origin'] == 'static') {
      $latitude = (float)$this->options['latitude'];
      $longitude = (float)$this->options['longitude'];
    }

    if ($this->options['origin'] == 'tied') {
      if (!empty($this->view->filter)) {
        foreach ($this->view->filter as $k => $v) {
          if ($v->table == 'location' && $v->field == 'distance' && $v->options['relationship'] == $this->options['relationship']) {
            if ($v->calculate_coords()) {
              $latitude = (float)$v->value['latitude'];
              $longitude = (float)$v->value['longitude'];
            }
          }
        }
      }
    }

    if (isset($latitude) && isset($longitude)) {
      return array('latitude' => $latitude, 'longitude' => $longitude);
    }
    else {
      return null;
    }
  }
}
