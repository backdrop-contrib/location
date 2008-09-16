<?php
// $Id$

/**
 * @file
 * Coordinates field handler.
 */

class location_handler_field_location_distance extends views_handler_field {

  function construct() {
    parent::construct();
    $this->additional_fields['longitude'] = 'longitude';
  }

  function option_definition() {
    $options = parent::option_definition();
    $options['origin'] = 'user';
    $options['units'] = 'km';
    $options['latitude'] = '';
    $options['longitude'] = '';
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

  function render($values) {
    $latitude = (float)$this->options['latitude'];
    $longitude = (float)$this->options['longitude'];

    if ($this->options['origin'] != 'static') {
      global $user;
      if (!isset($user->locations)) {
        $user = user_load($user->uid);
      }
      if (isset($user->locations[0]['latitude']) || !empty($user->locations[0]['latitude'])) {
        $latitude = (float)$user->locations[0]['latitude'];
        $longitude = (float)$user->locations[0]['longitude'];
      }
      else {
        if ($this->options['origin'] == 'user') {
          // Empty display.
          return;
        }
      }
    }

    if (empty($values->{$this->field_alias}) || empty($values->{$this->aliases['longitude']})) {
      // Unset location, empty display.
      return;
    }
    $dist = earth_distance($longitude, $latitude, $values->{$this->aliases['longitude']}, $values->{$this->field_alias});

    if ($this->options['units'] == 'km') {
      $dist = $dist / 1000.0;
      return theme('location_distance', $dist, 'km');
    }
    else {
      $dist = $dist / 1609.347;
      return theme('location_distance', $dist, 'mi');
    }
  }

}
