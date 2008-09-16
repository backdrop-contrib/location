<?php
// $Id$

/**
 * General proximity filter for location latitude/longitude.
 */
class location_views_handler_filter_proximity extends views_handler_filter {

  function option_definition() {
    $options = parent::option_definition();
    $options['type'] = 'latlon';

    $options['latitude'] = '';
    $options['longitude'] = '';
    $options['postal_code'] = '';
    $options['country'] = variable_get('location_default_country', 'us');
    $options['search_distance'] = 100;

    return $options;
  }

  function has_extra_options() {
    return TRUE;
  }

  function extra_options_form(&$form, &$form_state) {
    $form['type'] = array(
      '#prefix' => '<div class="views-left-40">',
      '#suffix' => '</div>',
      '#type' => 'radios',
      '#title' => t('Form mode'), // @@@ Less stupid title?
      '#options' => array(
        'latlon' => t('Latitude / Longitude input'),
        'postal' => t('Postal Code / Country'),
        'postal_default' => t('Postal Code (assume default country)'),
      ),
      '#description' => t('FIXME'),
      '#default_value' => $this->options['type'],
    );
  }

  function operator_options() {
    return array(
      'mbr' => t('Proximity (Rectangular)'),
      'dist' => t('Proximity (Circular)'),
    );
  }

  function value_form(&$form, &$form_state) {

    if ($this->options['type'] == 'latlon') {
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
    else if ($this->options['type'] == 'postal') {
      $form['postal_code'] = array(
        '#type' => 'textfield',
        '#title' => t('Postal code'),
        '#default_value' => $this->options['postal_code'],
      );
    }
    else {
      $form['postal_code'] = array(
        '#type' => 'value',
        '#value' => $this->options['postal_code'],
      );
    }

    $form['search_distance'] = array(
      '#type' => 'textfield', // @@@
      '#title' => t('Distance'),
      '#default_value' => $this->options['search_distance'],
    );
  }

  function exposed_form(&$form, &$form_state) {
    $key = $this->options['expose']['identifier'];
    if ($this->options['type'] == 'latlon') {
      $form[$key] = array(
        '#tree' => TRUE,
      );
      $form[$key]['latitude'] = array(
        '#type' => 'textfield',
        '#title' => t('Latitude'),
        '#default_value' => $this->options['latitude'],
      );
      $form[$key]['longitude'] = array(
        '#type' => 'textfield',
        '#title' => t('Longitude'),
        '#default_value' => $this->options['longitude'],
      );
    }

    $form[$key]['search_distance'] = array(
      '#type' => 'textfield', // @@@
      '#title' => t('Distance'),
      '#default_value' => $this->options['search_distance'],
    );

  }

/*
  function accept_exposed_input($input) {
    if (empty($this->options['exposed'])) {
      return TRUE;
    }

    if (!empty($this->options['expose']['use_operator']) && !empty($this->options['expose']['operator']) && isset($input[$this->options['expose']['operator']])) {
      $this->operator = $input[$this->options['expose']['operator']];
    }

    if (!empty($this->options['expose']['identifier'])) {
      $value = $input[$this->options['expose']['identifier']];

      // Various ways to check for the absence of optional input.
      if (!empty($this->options['expose']['optional'])) {
        if ($value == 'All' || $value === array()) {
          return FALSE;
        }

        if (!empty($this->no_single) && $value === '') {
          return FALSE;
        }
      }


      if (isset($value)) {
        $this->value = $value;
      }
      else {
        return FALSE;
      }
    }

    return TRUE;
  }
*/


  function query() {
    if (empty($this->value) || empty($this->value['latitude']) || empty($this->value['longitude'])) {
      return;
    }

    $this->ensure_my_table();

    // @@@ Zip

    $lat = $this->value['latitude'];
    $lon = $this->value['longitude'];
    $distance_float = (float)$this->value['search_distance'];

    $latrange = earth_latitude_range($lon, $lat, $distance_float); // @@@ Stupid parameter order, fix earth.inc?
    $lonrange = earth_longitude_range($lon, $lat, $distance_float);

    $this->query->add_where($this->options['group'], "$this->table_alias.latitude > %f AND $this->table_alias.latitude < %f AND $this->table_alias.longitude > %f AND $this->table_alias.longitude < %f", $latrange[0], $latrange[1], $lonrange[0], $lonrange[1]);

//  $query = 'SELECT n.nid, l.*, '. earth_distance_sql($latlon['lon'], $latlon['lat'], 'l') .' as distance FROM {node} n INNER JOIN {location_instance} i ON n.vid = i.vid INNER JOIN {location} l ON i.lid = l.lid
//WHERE l.latitude > %f AND l.latitude < %f AND l.longitude > %f AND l.longitude < %f AND '. earth_distance_sql($latlon['lon'], $latlon['lat']) .' < %f GROUP BY n.nid ORDER by distance';

  }
}
