<?php
/**
 * $Id$
 * @author Location Dev Team
 * @file location.theme.inc
 * Coordinates the theming
 * @todo Consider merge this file
 */
function theme_location_search_form(&$form) {
  $row = array();

  $row[] = array(
    'data' => drupal_render($form['proximity'])
  );

  $row[] = array(
    'data' => drupal_render($form['location'])
  );

  $output = theme('table', NULL, array($row), array('class' => 'location-search-form-table'));
  $output .= drupal_render($form);

  return $output;
}

function theme_location_map_link_options(&$form) {
  $header = array(array('align' => 'center',
    'data' => '<center>'. t('Country') .'</center>'),
    array('align' => 'center', 'data' => '<center>'. t('Options') .'</center>'));
  $rows = array();

  foreach (element_children($form['countries']) as $country_iso) {
    $row = array();

    $row[] = array(
      'data' => drupal_render($form['countries'][$country_iso]['label_'. $country_iso])
    );

    $row[] = array(
      'data' => drupal_render($form['countries'][$country_iso]['location_map_link_'. $country_iso])
    );

    $rows[] = $row;
  }

  $output = theme('table', $header, $rows);

  foreach (element_children($form) as $key) {
    $output .= drupal_render($form[$key]);
  }
  $output .= drupal_render($form);

  return $output;
}

function theme_location_geocoding_options(&$form) {
  $header = array(
    array('align' => 'center', 'data' => '<center>'. t('Country') .'</center>'),
    array('align' => 'center', 'data' => '<center>'. t('Options') .'</center>'),
    array('align' => 'center', 'data' => '<center>'. t('Configure') .'</center>')
  );

  $rows = array();

  foreach (element_children($form['countries']) as $country_iso) {
    $row = array();

    $row[] = array(
      'data' => drupal_render($form['countries'][$country_iso]['label_'. $country_iso])
    );

    $row[] = array(
      'data' => drupal_render($form['countries'][$country_iso]['location_geocode_'. $country_iso])
    );

    $row[] = array(
      'data' => drupal_render($form['countries'][$country_iso]['location_geocode_config_link_'. $country_iso])
    );

    $rows[] = $row;
  }

  $output = theme('table', $header, $rows);

  foreach (element_children($form) as $key) {
    $output .= drupal_render($form[$key]);
  }
  $output .= drupal_render($form);

  return $output;
}

/**
 * Generates HTML for the ALL passed locations.
 *
 * @param $location
 *   Array. A series of locations with the following fields:
 *      'street'       => A string representing the street location
 *      'additional'   => A string for any additional portion of the street location
 *      'city'         => A string for the city name
 *      'province'     => The standard postal abbreviation for the province
 *      'country'      => The two-letter ISO code for the country of the location (REQUIRED)
 *      'postal_code'  => The international postal code for the location
 * @param $hide
 *   Array. The values are the location fields to suppress in the themed display.
 * @ingroup themeable
 */
function theme_locations($locations = array(), $hide = array()) {
  $output = '';

  if (count($locations)) {
    $output .= '<h3>'. t('Location(s)') .'</h3>';
    foreach ($locations as $location) {
      $output .= theme('location', $location, $hide);
    }
  }

  return $output;
}

/**
 * Generates HTML for the passed location.
 *
 * @param $location
 *   Array. A single location with the following fields:
 *      'street'       => A string representing the street location
 *      'additional'   => A string for any additional portion of the street location
 *      'city'         => A string for the city name
 *      'province'     => The standard postal abbreviation for the province
 *      'country'      => The two-letter ISO code for the country of the location (REQUIRED)
 *      'postal_code'  => The international postal code for the location
 * @param $hide
 *   Array. The values are the location fields to suppress in the themed display.
 * @ingroup themeable
 */
function theme_location($location = array(), $hide = array()) {
  // Check if it have at least one field to be displayed.
  array_flip($hide);
  foreach (array('street', 'city', 'province', 'postal_code', 'country') as $field) {
    if (!empty($location[$field]) and empty($hide[$field])) {
      $display = TRUE;
      break;
    }
  }
  if (empty($display)) {
    return '';
  }

  $output = '';
  if (isset($location['country']) and ($f = theme_get_function('location_'. $location['country']))) {
    $output .= call_user_func($f, $location, $hide);
  }
  elseif (count($location)) {
    $output .= "\n";
    $output .= '<div class="location vcard"><div class="adr">'."\n";
    if (!empty($location['name']) and !in_array('name', $hide)) {
      $output .= theme('placeholder', $location['name']);
    }

    if (!empty($location['street']) and !in_array('street', $hide)) {
      $output .= '<div class="street-address">'. $location['street'];
      if (!empty($location['additional']) and !in_array('street', $hide)) {
        $output .= ' '. $location['additional'];
      }
      $output .='</div>';
    }

    if (!empty($location['city']) and !in_array('city', $hide)) {
      $city_province_postal[] = $location['city'];
    }

    if ((!empty($location['city']) and !in_array('city', $hide)) or
        (!empty($location['province']) and !in_array('province', $hide)) or
        (!empty($location['postal_code']) and !in_array('postal_code', $hide))) {

      $city_province_postal = array();

      if (!empty($location['city']) and !in_array('city', $hide)) {
        $city_province_postal[] = '<span class="locality">'. $location['city'] .'</span>';
      }

      if (!empty($location['province']) and !in_array('province', $hide)) {
        $city_province_postal[] = '<span class="region">'. $location['province'] .'</span>';
      }

      if (!empty($location['postal_code']) and !in_array('postal_code', $hide)) {
        $city_province_postal[] = '<span class="postal-code">'. $location['postal_code'] .'</span>';
      }

      $output .= implode(', ', $city_province_postal);
    }

    if (!empty($location['country']) and !in_array('country', $hide)) {
      $countries = _location_country_get();
      $output .= '<div class="country-name">'. t($countries[$location['country']]) .'</div>';
    }

    if (isset($location['latitude']) and isset($location['longitude'])) {
      $output .=  '<div class="geo"><abbr class="latitude" title="'. $location['latitude'] .'"></abbr><abbr class="longitude" title="'. $location['latitude'] .'" ></abbr></div>';
    }

    $output .= '</div></div>';
  }

  $output .= location_map_link($location);

  return $output;
}

/**
 * Theme the Location Address form
 *
 * @param &$form
 *   Array. The form structure
 * @ingroup themeable
 */
function theme_location_address_form(&$form) {
  // If the country and state fields are present,
  // enable the extra JS
  if ($form[$field_name]['#type'] != 'hidden' and $form['country']['#type'] != 'hidden') {
    drupal_add_js(drupal_get_path('module', 'location') .'/location.js');
  }

  // The required field notation
  $req_str = '<span class="form-required" title="'. t('This field is required.') .'">*</span>';

  foreach (element_children($form) as $field_name) {
    $col = array();
    if ($form[$field_name]['#type'] == 'markup') {
      $col[] = array('data' => $form[$field_name]['#value'], 'colspan' => 2);
    }
    elseif ($form[$field_name]['#type'] != 'hidden') {
      $required = !empty($form[$field_name]['#required']) ? $req_str : '';

      // Check if the element has a title
      if (!empty($form[$field_name]['#title'])) {
        $col[] = array(
          'align' => 'right',
          'data'  => '<div class="form-item"><label'. ($form[$field_name]['#id'] ?
          ' for="'. $form[$field_name]['#id'] .'" ' : '') .'>'.
          filter_xss_admin($form[$field_name]['#title']) .": $required</label></div>"
        );
        unset($form[$field_name]['#title']);
      }
      else {
        $col[] = array('');
      }

      // The field itselt
      $col[] = array(
        'align' => 'left',
        'data' => drupal_render($form[$field_name])
      );

      $rows[] = array(
        'data'  => $col,
        'class' => 'odd'
      );
    }
  }

  $output = theme('table', NULL, $rows);
  $output .= drupal_render($form);

  return $output;
}

/**
 * List all addresses from a given user
 *
 * @param $uid
 *   Number. The user ID
 * @param $addresses
 *   Array. A list of user addresses
 * @ingroup themeable
 */
function theme_location_address_overview($uid, $addresses) {
  $output = "<p>". l(t('Click here to add a new address'),
    "user/$uid/location-address/add") .'.</p>';

  $countries = _location_country_get();

  foreach ($addresses as $address) {
    $addr = array();
    $addr[] = theme('placeholder', $address->name);
    $addr[] = check_plain($address->street);
    $addr[] = ($address->additional) ? check_plain($address->additional) : '';
    $addr[] = check_plain($address->city) .', '. check_plain($address->province) .', '.
      $countries[check_plain($address->country)];
    $addr[] = check_plain($address->postal_code);
    foreach ($addr as $k => $v) {
      if (empty($v)) {
        unset($addr[$k]);
      }
    }
    $row[] = array(implode('<br />', $addr), l(t('edit'),
      "user/$uid/location-address/edit/$address->lid") ." / ". l(t('delete'),
      "user/$uid/location-address/delete/$address->lid"));
  }
  if (!empty($row)) {
    $header = array(t('address'), t('options'));
    $output .= theme('table', $header, $row);
  }

  return $output;
}

function theme_location_distance_proximity_form(&$form) {
  $row = array();

  $row[] = array(
    'data' => t('Search within') .' '
  );

  $row[] = array(
    'data' => drupal_render($form['distance']),
    'cellspacing' => 0,
    'cellpadding' => 0
  );

  if ($form['unit']['#type'] == 'select') {
    $row[] = array(
      'data' => drupal_render($form['unit']),
      'cellspacing' => 0,
      'cellpadding' => 0
    );

    $row[] = array(
      'data' => ' '. t('of:'),
      'cellspacing' => 0,
      'cellpadding' => 0
    );
  }
  else {
    $row[] = array(
      'data' => ($form['unit']['#value'] == 'km' ? t('km') : t('miles')) .' '. t('of:') . drupal_render($form['unit']),
      'cellspacing' => 0,
      'cellpadding' => 0
    );
  }

  $output = theme('table', NULL, array($row));
  $output .= drupal_render($form);

  return $output;
}
