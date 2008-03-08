<?php
// $Id$

/**
 * @file
 * An implementation of a universal API for location manipulation.  Provides functions for
 * postal_code proximity searching, deep-linking into online mapping services.  Currently,
 * some options are configured through an interface provided by location.module.
 */

define('LOCATION_PATH', drupal_get_path('module', 'location'));

include_once LOCATION_PATH.'/earth.inc';

/**
 * Get a deep-link to a mapping service such as Yahoo! Maps or MapPoint given an location.  The
 * call is delegated based on the 'country' value in the $location parameter.
 *
 * @param $location
 *   An associative array where
 *      'street'       => A string representing the street location
 *      'additional'   => A string for any additional portion of the street location
 *      'city'         => A string for the city name
 *      'province'     => The standard postal abbreviation for the province
 *      'country'      => The two-letter ISO code for the country of the location (REQUIRED)
 *      'postal_code'  => The international postal code for the location
 *
 * @return
 *   A link to a map provided by a third-party.  The idea is to encode the appropriate
 *   parameters as HTTP GET variables to the URL.
 */
function location_map_link($location = array(), $link_text = 'See map: ') {
  if (!isset($location['country']) && $location['country'] != 'xx') {
    return '';
  }

  $default_func = 'location_map_link_'. $location['country'] .'_default_providers';
  $providers_func = 'location_map_link_'. $location['country'] .'_providers';
  $providers = function_exists($providers_func) ? $providers_func() : array();
  $selected_providers = variable_get('location_map_link_'. $location['country'], function_exists($default_func) ? $default_func() : array());

  $links = array();
  foreach ($selected_providers as $mapper) {
    $link_func = 'location_map_link_'. $location['country'] .'_'. $mapper;
    if (function_exists($link_func)) {
      if ($link = $link_func($location)) {
        $links[] = '<a href="'. $link .'">'. $providers[$mapper]['name'] .'</a>';
      }
    }
  }
  if (count($links)) {
    return t($link_text) . implode($links, ", ");
  }
  else {
    return NULL;
  }
}

/**
 * Try to extract the the Latitude and Longitude data from the
 * postal code.
 *
 * @param $location
 *   Array. the location data
 *   -> the values are:
 *     'street'         => the string representing the street location (REQUIRED)
 *     'additional'     => the string representing the additional street location portion in the location form
 *     'city'           => the city name (REQUIRED)
 *     'province'       => the province code defined in the country-specific include file
 *     'country'        => the lower-case of the two-letter ISO code (REQUIRED)
 *     'postal_code'    => the postal-code (REQUIRED)
 *
 * @return
 *   Array or NULL. NULL if the delegated-to function that does the
 *   actual look-up does not exist. If the appropriate function exists,
 *   then this function returns an associative array where
 *    'lon' => A floating point number for the longitude coordinate of the parameter location
 *    'lat' => A floating point number for the latitude coordinate of the parameter location
 */
function location_get_postalcode_data($location = array()) {
  //@@@ Megapatch change here.
  $location['country'] = isset($location['country']) ? trim($location['country']) : NULL;
  $location['postal_code'] = isset($location['postal_code']) ? trim($location['postal_code']) : NULL;
  if (is_null($location['postal_code']) || is_null($location['country']) || empty($location['country']) || empty($location['postal_code']) || $location['postal_code'] == 'xx') {
    return NULL;
  }
  $country_specific_function = 'location_get_postalcode_data_'. $location['country'];
  if (function_exists($country_specific_function)) {
    return $country_specific_function($location);
  }
  else {
    return NULL;
  }
}

function _location_is_empty($location = array()) {
  $fields = array('street', 'additional', 'city', 'province', 'postal_code', 'country');
  foreach ($fields as $field) {
    if (isset($location[$field]) && strlen(trim($location[$field]))) {
      return FALSE;
    }
  }
  return TRUE;
}

/**
 * Given two points in lat/lon form, returns the distance between them.
 *
 * @param $latlon_a
 *   An associative array where
 *      'lon' => is a floating point of the longitude coordinate for the point given by latlonA
 *      'lat' => is a floating point of the latitude coordinate for the point given by latlonB
 *
 * @param $latlon_b
 *      Another point formatted like $latlon_b
 *
 * @param $distance_unit
 *      A string that is either 'km' or 'mile'.
 *      If neither 'km' or 'mile' is passed, the parameter is forced to 'km'
 *
 * @return
 *    NULL if sense can't be made of the parameters.
 *    An associative array where
 *      'scalar' => Is the distance between the two lat/lon parameter points
 *      'distance_unit' => Is the unit of distance being represented by 'scalar'.
 *                         This will be 'km' unless 'mile' is passed for the $distance_unit param
 */
function location_distance_between($latlon_a = array(), $latlon_b = array(), $distance_unit = 'km') {
  if (!isset($latlon_a['lon']) || !isset($latlon_a['lat']) || !isset($latlon_b['lon']) || !isset($latlon_b['lat'])) {
    return NULL;
  }

  if ($distance_unit != 'km' && $distance_unit != 'mile') {
    return NULL;
  }

  // $conversion_factor = number to divide by to convert meters to $distance_unit
  // At this point, $distance_unit == 'km' or 'mile' and nothing else
  //$conversion_factor = ($distance_unit == 'km') ? 1000.0 : 1609.347;

  $meters = earth_distance($latlon_a['lon'], $latlon_a['lat'], $latlon_b['lon'], $latlon_b['lat']);
  return array('scalar' => round($meters/(($distance_unit == 'km') ? 1000.0 : 1609.347), 1), 'distance_unit' => $distance_unit);
}

/**
 * Generates a Drupal HTML form for collecting locations.
 *
 * @param $fields
 *   An array of values where each value is one of 'street', 'city', 'province', 'postal_code', or 'country'.
 *   The presence of values in this array determines which fields will be served in the location form generated
 *   by a call to this function.  If this array is empty, all fields are generated.
 *
 * @param $prefilled_values
 *   An associative array where
 *     -> Each key is one of the location fields: 'street', 'additional', 'city', 'province', 'postal_code', 'country'
 *     -> Each value is a prefilled value for the given field.
 *
 * @param $required_fields
 *   An array of values that are required.  Each string can be one of 'street', 'city', 'postal_code', 'province', or 'country'.
 *   The presence of values in this array determines which fields will be marked as 'required'.  Validation (i.e., making sure
 *   a required value is actually filled in is the responsibility of the caller)
 *
 * @param $suppressed_values
 *   An array of values that are to be automatically filled and hidden from user view.  These will be indicated in this
 *   associative array with the following possibilities for keys:
 *     'province' => The standard province value as defined keyed by the country specific file (e.g., for US states, its the capitalized two letter abbreviation.
 *     'country' => The lower-case two letter ISO code for the country being assumed.
 *
 * @param $description
 *   A text description of specifically what location is being collected by the form to be served.
 *
 * @param $form_name
 *   An additional parameter to help prevent HTML input name collisions.  If the caller is using this
 *   function to generate more than 1 location form on a page, then the generated name for each HTML input's
 *   "name" attribute will go by the value supplied for $form_name.  This parameter is defaulted to 'location'.
 *   For example, if $form_name == 'xyz' and there is a 'street' field in the form to be served,
 *   the "name" attribute for the HTML <input> will be "edit[xyz][street]"
 *
 * @param $function
 *   A string that tells location_form() which location API function will be using the location submitted via the
 *   generated form.  For example, if $function == 'latlon_rough', then the returned location_form (if it includes
 *   a country field) will only generate a list of countries in the HTML select for which function location_latlon_rough()
 *   is supported.  To figure out which countries these are, we check to see which of the configured countries have existing
 *   functions to support the call.  In this case, we would check to see if there existed a function called "location_latlon_rough_us()"
 *   before listing the United States in the HTML SELECT for the generated location form.  $function is defaulted to NULL.
 *   If $function is NULL, the HTML SELECT that is generated will list all countries.
 *
 * @return
 *   An location form based on the parameters specified.  If the $fields array is empty, then the
 *   function returns a form in which all possible fields are served as optional form items.
 *
 * @ingroup form
 *
 * EXAMPLES:
 *
 *    -> The following call returns a form that only contains fields for a postal_code and country where
 *       the postal_code is required:
 *            ---
 *            $form = location_form(array('postal_code', 'country'), array(), array('postal_code', 'country'), 'Permanent location')
 *            ---
 *       The form returned by this call is generated with calls to Drupal's 'form_' functions:
 *
 *            $form  = form_textfield('Postal Code', 'location][postal_code', '', 64, 64, NULL, NULL, TRUE);
 *       -------------------------------------------------------------------------------------------------
 *    -> The following call returns a form that contains fields for a street, province, and postal_code,
 *       but the street, city, and province fields are optional while postal_code is required:
 *            ---
 *            $form = location_form(array('street', 'city', 'province', 'postal_code'), array(), array('postal_code'));
 *            ---
 *    -> The form returned by this call is generated with the following calls to Drupal's 'form_' functions:
 *
 *            $form  = form_textfield('Street', 'location][street', '', 64, 64);
 *            $form .= form_textfield('Additional', 'location][additional', '', 64, 64);
 *            // The 'Additional' field is always and only generated when 'street' is specified as one of the fields.
 *            // The 'Additional' field is always optional whether or not 'Street' is required.
 *            $form .= form_textfield('City', 'location][city', '', 64, 64);
 *            $form .= _location_province_select_options(); // defined below
 *            $form .= form_textfield('Postal Code', 'location][postal_code', '', 64, 64, NULL, NULL, TRUE);
 *        ------------------------------------------------------------------------------------------------
 *    For the following examples, assume we have the following two locationes:
 *       (1) $location_a = ('street' => '2010 Broadway St', 'city' => 'Redwood City', 'province' => 'CA', 'postal_code' => '94063', 'country' => 'us');
 *       (2) $location_b = ('street' => '2010 Broadway St', 'city' => 'Redwood City', 'province' => 'us-CA', 'postal_code' => '94063', 'country' => 'us');
 *     -> The following calls return the exact same form that contains fields for a street, province, postal_code, where prefilled
 *        values are submitted to the form.
 *
 *            $form = location_form(array('street', 'city', 'province', 'postal_code', 'country'), $location_b, array('street', 'city', 'province', 'postal_code', 'country'), '', 'home_location');
 *
 *            $form = location_form(array('street', 'city', 'province', 'postal_code', 'country'), location_api2form($location_a), array('street', 'city', 'province', 'postal_code', 'country'), '', 'home_location');
 *
 *     -> The form returned by these call is ultimately generated with the following calls to Drupal's 'form_' functions:
 *
 *            $form  = textfield('Street', 'home_location][street', '2010 Broadway St.', 64, 64, NULL, NULL, TRUE);
 *            $form .= textfield('Additional', 'home_location][additional', 'Attn: Ankur Rishi', 64, 64, NULL, NULL, TRUE);
 *            $form .= textfield('City', 'home_location][city', 'Redwood City', 64, 64, NULL, NULL, TRUE);
 *            $form .= _location_province_select_options(TRUE, 'us-CA', 'home_location');
 *            $form .= textfield('Postal Code', 'home_location][postal_code', '94063', 64, 64, NULL, NULL, TRUE);
 *            $form .= _location_country_select_options(TRUE, 'us', 'home_location');
 *
 *        Note that in both cases, the first and third argument can take the same array since all the fields are being required.
 */
function location_form($fields = array(), $prefilled_values = array(), $required_fields = array(), $suppressed_values = array(), $description = '', $function = NULL) {
  // If no field was specified, use all fields.
  if (empty($fields)) {
    $fields = array('name', 'street', 'city', 'province', 'postal_code', 'country');
  }

  $form = array();
  $form['description'] = array(
    '#type'   => 'markup',
    '#value'  => $description
  );

  if (in_array('name', $fields)) {
    $form['name'] = array(
      '#type'           => 'textfield',
      '#title'          => t('Location name'),
      '#default_value'  => isset($prefilled_values['name']) ? $prefilled_values['name'] : '',
      '#size'           => 64,
      '#maxlength'      => 64,
      '#description'    => t('e.g. a place of business, venue, meeting point'),
      '#attributes'     => NULL,
      '#required'       => in_array('name', $required_fields)
    );
  }
  if (in_array('street', $fields)) {
    $form['street'] = array(
      '#type'           => 'textfield',
      '#title'          => t('Street'),
      '#default_value'  => isset($prefilled_values['street']) ? $prefilled_values['street'] : '',
      '#size'           => 64,
      '#maxlength'      => 64,
      '#required'       => in_array('street', $required_fields)
    );
    $form['additional'] = array(
      '#type'           => 'textfield',
      '#title'          => t('Additional'),
      '#default_value'  => isset($prefilled_values['additional']) ? $prefilled_values['additional'] : '',
      '#size'           => 64,
      '#maxlength'      => 64
    );
  }
  if (in_array('city', $fields)) {
    $form['city'] = array(
      '#type'           => 'textfield',
      '#title'          => t('City'),
      '#default_value'  => isset($prefilled_values['city']) ? $prefilled_values['city'] : '',
      '#size'           => 64,
      '#maxlength'      => 64,
      '#description'    => NULL,
      '#attributes'     => NULL,
      '#required'       => in_array('city', $required_fields)
    );
  }
  if (in_array('province', $fields)) {
    if (in_array('province', array_keys($suppressed_values))) {
      $form['province'] = array(
        '#type'             => 'hidden',
        '#value'            => $suppressed_values['province']
      );
    }
    else {
      $form['province'] = _location_province_select_options(isset($prefilled_values['province']) ? $prefilled_values['province'] : '', in_array('province', $required_fields), in_array('country', array_keys($suppressed_values)) ? $suppressed_values['country'] : NULL);
    }
  }
  if (in_array('postal_code', $fields)) {
    $form['postal_code'] = array(
      '#type'           => 'textfield',
      '#title'          => t('Postal code'),
      '#default_value'  => isset($prefilled_values['postal_code']) ? $prefilled_values['postal_code'] : '',
      '#size'           => 16,
      '#maxlength'      => 16,
      '#required'       => in_array('postal_code', $required_fields)
    );
  }
  if (in_array('country', $fields)) {
    if (in_array('country', array_keys($suppressed_values))) {
      $form['country'] = array(
        '#type'   => 'hidden',
        '#value'  => $suppressed_values['country']
      );
    }
    else {
      $form['country'] = _location_country_select_options(isset($prefilled_values['country']) ? $prefilled_values['country'] : '', in_array('country', $required_fields), $function);
    }
  }

  $form['#theme'] = 'location_form';

  return $form;
}


// @@@ Update for newer gmap
function location_latlon_form($description = '', $prefilled_values = array()) {
  $form = array();

  $usegmap = (function_exists('gmap_set_location') && variable_get('location_usegmap', FALSE));

  if ($usegmap) {
    $form['map'] = array();  //reserve spot at top of form for map
  }

  $form['latitude'] = array(
      '#type' => 'textfield',
      '#title' => t('Latitude'),
      '#default_value' => isset($prefilled_values['latitude']) ? $prefilled_values['latitude'] : '',
      '#size' => 64,
      '#maxlength' => 64
      );
  $form['longitude'] = array(
      '#type' => 'textfield',
      '#title' => t('Longitude'),
      '#default_value' => isset($prefilled_values['longitude']) ? $prefilled_values['longitude'] : '',
      '#size' => 64,
      '#maxlength' => 64,
      '#description' => $description,
      );

  if ($usegmap) {
    $map_macro = variable_get('gmap_user_map', '[gmap|id=usermap|center=0,30|zoom=16|width=100%|height=400px]');
    $form['map']['gmap']['#value'] = gmap_set_location($map_macro, $form, array('latitude' => 'latitude', 'longitude' => 'longitude'));
  }
  return $form;
}

/**
 * @param $location
 *   An associative array that has been submitted by an HTML form generated by location_form().
 *
 * @return
 *   An associative array in which the submitted values are modified to pass to the location API
 *   as the $location parameter (excepting location_form()).
 *
 *   This means changing the province field to remove the country code and dash.
 *   For example: California is served by the key 'us-CA' in the location form and this is what is passed when it is
 *                submitted through a form generated by location_form().
 *
 *                This is changed to 'CA' in the returned array.
 */
function location_form2api($location = array()) {
  // @@@ Differs from megapatch.
  // The user may have selected a state/province, but did not specify a country
  if (isset($location['province']) && strlen($location['province']) && $location['province'] != 'xx') {
    if (!isset($location['country']) || (isset($location['country']) && !strlen($location['country']))) {
      $location['country'] = substr($location['province'], 0, 2);
    }
  }

  $translated = array();
  $location = is_array($location) ? $location : array();
  foreach ($location as $key => $value) {
    $value = trim($value);
    if ($key == 'province') {
      if (strlen($value) && $value != 'xx') {
        // chop off the 2-letter code and the '-' from the front of the value
        $value = substr($value, 3);
      }
    }
    $translated[$key] = $value;
  }
  return $translated;
}

/**
 * Inverse of location_form2api()
 *
 * @param $location
 *   An associative array that can be passed as the $location parameter to the location API.
 *
 * @return
 *   An associative array with the same values modified so that the array can be passed as the
 *   $prefilled_values parameter to location_api2form()
 *
 *   Meant to take the standard location array format used by the public API (minus the form generating functions) and convert them
 *   into values that can be used by location_form() to fill in the prefilled values.
 */
function location_api2form($location = array()) {
// @@@ FAPI
  $translated = array();
  foreach ($location as $key => $value) {
    if ($key == 'province') {
      if (strlen($value) && isset($location['country']) && strlen($location['country']) && $location['province'] != 'xx' && $location['country'] != 'xx') {
        $translated['province'] = $location['country'] .'-'. $value;
      }
    }
    else {
      $translated[$key] = $value;
    }
  }
  return $translated;
}

/**
 * This function generates a form for doing proximity searches within a certain distance
 * of a specified point.
 *
 * Depending on the context within which this function is called, the search-point can either
 * be user-supplied via the location form that is passed (if one is available) or done within
 * a search-point extracted from a contact table or some other location source specified by
 * the programmer calling this function.
 *
 * @param $prefilled_values
 *   An associative array for prefilled values for the proximity search parameters, where
 *     'distance' => is the prefilled int value to be selected for the distance scalar
 *     'distance_unit' => is 'km' or 'mile'
 *
 * @param $suppressed_values
 *   An associative array for values you wish to force the selection of rather than pre-fill as a default.
 *   The value will be passed as a hidden form input.  The passed values will be taken in as an associative
 *   array where
 *     'distance' => a preselected positive integer for the distance
 *     'distance_unit' => a preselected unit for the distance, one of either 'km' or 'mile'
 *
 * @return
 *   An HTML form (generated by Drupal form functions) that lets users specify proximity search parameters that include distance,
 *   the unit of distance, and a search-point if the optional $location_form parameter is passed.  If one is not passed,
 *   the caller of this function will be assumed to already have one.
 *
 */
function location_proximity_form($prefilled_values = array(), $suppressed_values = array()) {
  $form = array();

  if (in_array('distance', array_keys($suppressed_values))) {
    $form['distance'] = array(
      '#type' => 'hidden',
      '#value' => $suppressed_values['distance']
    );
  }
  else {
    $form['distance'] = array(
      '#type' => 'select',
      '#default_value' => (isset($prefilled_values['distance']) ? $prefilled_values['distance'] : 25),
      '#options' => drupal_map_assoc(array(5, 10, 25, 50, 100, 250)),
    );
  }

  if (in_array('unit', array_keys($suppressed_values))) {
    $form['unit'] = array(
      '#type' => 'hidden',
      '#value' => $suppressed_values['unit']
    );
  }
  else {
    $form['unit'] = array(
      '#type' => 'select',
      '#default_value' => (isset($prefilled_values['unit']) ? $prefilled_values['unit'] : 'mile'),
      '#options' => array('mile' => 'miles', 'km' => 'km'),
    );
  }

  $form['#theme'] = 'location_proximity_form';

  return $form;
}

/**
 * A helper function that tells you whether all the fields of an location are full (excluding the 'additional' field.
 *
 * @param $location
 *   An associative array where
 *     -> The keys are the different field names of an location: 'street', 'additional', 'city', 'province', 'postal_code', and 'country'
 *
 * @return
 *   TRUE if there is a non-empty (trimmed) string for the following keys' values: 'street', 'city', 'province', 'postal_code', 'country'.
 *   FALSE otherwise.
 */
function _location_is_full($location = array()) {
  if (!isset($location['street']) && trim($location['street']) != '') {
    return FALSE;
  }

  if (!isset($location['city']) && trim($location['city']) != '') {
    return FALSE;
  }

  if (!isset($location['province']) && trim($location['province']) != '') {
    return FALSE;
  }

  if (!isset($location['postal_code']) && trim($location['postal_code']) != '') {
    return FALSE;
  }

  if (!isset($location['country']) && trim($location['country']) != '') {
    return FALSE;
  }

  return TRUE;
}

/**
 * Takes two locationes and tries to return a deep-link to driving directions.
 *
 * Parameters:
 * @param $location_a
 *   An associative array that represents an location where
 *      'street'       => the street portions of the location
 *      'additional'   => additional street portion of the location
 *      'city'         => the city name
 *      'province'     => the province, state, or territory
 *      'country'      => lower-cased two-letter ISO code (REQUIRED)
 *      'postal_code'  => the postal code
 *
 * @param $location_b
 *   An associative array that represents an location in the same way that
 *   parameter $location_a does.
 *
 * @param $link_text
 *   The text of the HTML link that is to be generated.
 *
 * @return
 *   A deep-link to driving directions on Yahoo! or some other mapping service, if enough fields are filled in the parameters.
 *   A deep-link to a form for driving directions with information pre-filled if not enough, but some fields are filled in the parameters.
 *   The empty string if no information is provided (or if so little information is provided that there is no function to which to delegate
 *   the call.
 *
 *   We dispatch the call to a country-specific function.  The country-specific function, in this case,
 *   will be the one reflected by the country parameter of the first function.  We require that
 *   both locationes supplied have a country field at the minimum.
 *
 *   The country-specific functions will ultimately decide, with the parameters given, whether to
 *   to link to a form for driving directions is provided, where this form will be
 *   pre-populated with whatever values were available or whether to link directly to the driving
 *   directions themselves if enough fields are filled for each location.
 */
function location_driving_directions_link($location_a = array(), $location_b = array(), $link_text = 'Get directions') {
  if (!isset($location_a['country']) or !isset($location_b['country'])) {
    return '';
  }

  // For now, return empty string if starting-point and destinations are in different countries
  //if ($location_a['country'] != $location_b['country']) {
  //  return '';
  //}
  // Lines above commented out because I want to let the country-specific function of the departure point decide
  // what it will do with driving destination locationes from other countries.  As an example, Yahoo! Maps supports driving
  // direction queries for locationes between the U.S. and Canada.

  $driving_direction_function = 'location_driving_directions_link_'. $location_a['country'];
  if (function_exists($driving_direction_function)) {
    $http_link = $driving_direction_function($location_a, $location_b);
    if (strlen($http_link)) {
      return '<a href="'. $http_link .'">'. $link_text .'</a>';
    }
    else {
      return '';
    }
  }

  return '';
}

/**
 * Takes an location and a distance and returns an array of all postal-codes (from all countries that are supported)
 * within the specified distance of the specified location.
 *
 * @param $location
 *   An associative array where
 *     'street'       => the street location
 *     'additional'   => extra street location or building name or hall or something like that.\
 *     'city'         => a city name
 *     'province'     => province code as defined by the country specific include file
 *     'country'      => lower-cased two-letter ISO 3166 code (REQUIRED)
 *     'postal_code'  => the postal_code
 *
 * @param $distance
 *   The number portion of the distance; it is forced to an integer ceiling
 *
 * @param $distance_unit
 *   The unit of distance being used for the distance being submitted.
 *   Valid arguments for $distance_unit are 'mile' and 'km'.  If something other than one of
 *   these unit types is submitted for this argument, it is forced to 'km'.
 *
 * @return
 *   An array where
 *     -> the keys are a postive integer ranking of the search result's closeness to the parameter $postal_code
 *        with 1 being assigned to the nearest postal code
 *     -> the values are an associative array where
 *          'postal_code'   => A postal code that fell within the search-radius given by $distance and $distance_unit.
 *          'country'       => The two-letter ISO code for the home-country of this particular postal_code search result.
 *          'city'          => The city to which this postal code belongs.
 *          'province'      => The province to which this postal code belongs.
 *          'lon'           => The longitude coordinate of the approximate center of the area covered by 'postal_code'
 *          'lat'           => The latitude coordinate of the approximate center of the area covered by 'postal_code'
 *          'distance'      => The number of 'km's or 'mile's that are between the approximate center of the area of
 *                             the $postal_code parameter and that of the 'postal_code' in this subarray
 *          'distance_unit' => The unit of distance specified by 'scalar'
 */
function location_nearby_postalcodes_bylocation($location, $distance, $distance_unit = 'km') {
  $latlon = location_latlon_rough($location);

  // If we could not get lat/lon coordinates for the given location, return an empty search result set.
  if (!isset($latlon['lat']) || !isset($latlon['lon'])) {
    return array();
  }

  // If the distance parameters did not make sense, return an empty search result set.
  if (!($distance_float = _location_convert_distance_to_meters($distance, $distance_unit))) {
    return array();
  }

  $search_results = _location_nearby_postalcodes($latlon['lon'], $latlon['lat'], $distance_float);
  _location_format_search_result_distances($search_results, $distance_unit);
  return $search_results;
}

/**
 * Takes a latitude, longitude, and a distance, and returns all postal_codes within
 *
 * @param $lon
 *   A floating point of the longitude coordinate of the search point
 *
 * @param $lat
 *   A floating point of the latitude coordinate of the search point
 *
 * @param $distance
 *   The number portion of the distance; it is forced to an integer ceiling
 *
 * @param $distance_unit
 *   The unit of distance being used for the distance being submitted.
 *   Valid arguments for $distance_unit are 'mile' and 'km'.  If something other than one of
 *   these unit types is submitted for this argument, it is forced to 'km'.
 *
 * @return
 *   An array where
 *     -> the keys are a contatenation of the country's ISO code and the postal code.  For example, if
 *        one of the search results is for postal code "94803" in the United States, the key is then "us94803"
 *     -> the values are an associative array where
 *          'postal_code'   => A postal code that fell within the search-radius given by $distance and $distance_unit.
 *          'country'       => The two-letter ISO code for the home-country of this particular postal_code search result.
 *          'lon'           => The longitude coordinate of the approximate center of the area covered by 'postal_code'
 *          'lat'           => The latitude coordinate of the approximate center of the area covered by 'postal_code'
 *          'distance'      => The number of 'km's or 'mile's that are between the approximate center of the area of the
 *                             $postal_code parameter and that of the 'postal_code' in this array
 *          'distance_unit' => The unit of distance specified by 'distance'
 */
function location_nearby_postalcodes_bylatlon($lon, $lat, $distance, $distance_unit = 'km') {
  // If the given $lon/$lat don't make sense, return an empty search result set.
  if (!is_numeric($lon) || !is_numeric($lat)) {
    return array();
  }

  // If the distance parameters did not make sense, return an empty search result set.
  $distance_float = _location_convert_distance_to_meters($distance, $distance_unit);
  if (is_null($distance_float)) {
    return array();
  }

  $search_results = _location_nearby_postalcodes($lon, $lat, $distance_float);
  _location_format_search_result_distances($search_results, $distance_unit);

  return $search_results;
}

/**
 * @param $distance
 *   A number in either miles or km.
 *
 * @param $distance_unit
 *   String (optional). Either 'mile' or 'km' (default)
 *
 * @return
 *   A floating point number where the number in meters after the initially passed scalar has been ceil()'d
 *   This is done after the $distance_unit parmeter is forced to be 'km' or 'mile'
 */
function _location_convert_distance_to_meters($distance, $distance_unit = 'km') {
  if (!is_numeric($distance)) {
    return NULL;
  }

  // Force an integer version of distance, just in case anyone wants to add a caching mechanism
  // for postal code proximity searches.

  if (is_float($distance)) {
    $distance = intval(ceil($distance));
  }

  if ($distance < 1) {
    return NULL;
  }

  if ($distance_unit != 'km' && $distance_unit != 'mile') {
    $distance_unit = 'km';
  }

  // Convert distance to meters
  //$distance_float = floatval($distance) * (($distance_unit == 'km') ? 1000.0 : 1609.347);
  //return round($distance_float, 2);
  $retval = round(floatval($distance) * (($distance_unit == 'km') ? 1000.0 : 1609.347), 2);
  return $retval;
}

/**
 * Takes an location and returns a "rough" latitude/longitude pair based on the postal code
 * data available for the given country.
 *
 * @param $location
 *   An associative array $location where
 *     'street'       => the street portion of the location
 *     'additional' => additional street portion of the location
 *     'province'     => the province, state, or territory
 *     'country'      => lower-cased two-letter ISO code (REQUIRED)
 *     'postal_code'  => international postal code (REQUIRED)
 *
 * @return
 *   NULL if data cannont be found.
 *   Otherwise, an associative array where
 *     'lat' => is a floating point of the latitude coordinate of this location
 *     'lon' => is a floating point of the longitude coordinate of this location
 */
function location_latlon_rough($location = array()) {
  if (!isset($location['country']) || !isset($location['postal_code'])) {
    return NULL;
  }

  $latlon_function = 'location_latlon_rough_'. $location['country'];
  if (function_exists($latlon_function)) {
    return $latlon_function($location);
  }
  else {
    return NULL;
  }
}

/**
 * This is the main logic-level function for performing proximity postal-code searches.
 * It calls a number of helper functions for finding postal_code results in each country,
 * narrowing down results, formatting the returned array, and sorting the returned array.
 * Finally, it implements a caching mechanism that will return a subset (proper or not)
 * of a previous search's results on the same search-point.
 *
 * @param $lon
 *   A floating point of the longitude coordinate about which the search is being executed
 *
 * @param $lat
 *   A floating point of the latitude coordinate about which the search is being executed
 *
 * @param $distance
 *   A floating point of the distance in meters; it is forced to an integer ceiling, but
 *   kept as a float
 *
 * @return
 *   An array where
 *     -> the keys are a concatenation of the lower-case two-letter ISO country code and the postal code
 *        For example, 94063 in the United States would be 'us94063'
 *     -> the values are an associative array where
 *          'postal_code' => A postal code that fell within the search-radius given by $distance and $distance_unit.
 *          'country'     => The two-letter ISO code for the home-country of this particular postal_code search result.
 *          'lon'         => The longitude coordinate of the approximate center of the area covered by 'postal_code'
 *          'lat'         => The latitude coordinate of the approximate center of the area covered by 'postal_code'
 *          'distance'    => The floating point distance of the approximate center of this 'postal_code' from the
 *                           parameter lat/lon point in meters
 *
 */
function _location_nearby_postalcodes($lon, $lat, $distance) {
  $search_results = _location_search_results_from_cache($lon, $lat, $distance);

  // If there were usable cached search_results then return those and go no further... awwwww yeah.
  if (count($search_results)) {
    return $search_results;
  }
  //------------------------------------------------------------------------------------------
  // Pulling from the cache takes place right here.
  // The cache id ("cid", to be inserted into the {cache_location} table) will be a concatenation of
  //   -> "location_prox_search:".
  //   -> round($lon, 3) . ':'.
  //   -> round($lat, 3) . ":".
  //   -> $distance
  // The value of the cached item will be a serialize($result_array)
  //
  // The cache will be cleared of proximity searches when there is a change in countries that have
  // been configured into the system.
  //------------------------------------------------------------------------------------------

  $search_results = array();

  $latrange = earth_latitude_range($lon, $lat, $distance);
  $lonrange = earth_longitude_range($lon, $lat, $distance);

  $result = db_query('SELECT zip, city, state, country, '. earth_distance_sql($lon, $lat) .' as distance  FROM {zipcodes} WHERE latitude > %f AND latitude < %f AND longitude > %f AND longitude < %f AND '. earth_distance_sql($lon, $lat) .' < %f ORDER by distance', $latrange[0], $latrange[1], $lonrange[0], $lonrange[1], $distance);

  while ($result_row = db_fetch_object($result)) {
    $search_results[$result_row->country . $result_row->zip] = array('city' => $result_row->city, 'province' => $result_row->state, 'distance' => $result_row->distance);
  }

  cache_set('location_prox_search:'. round($lon, 3) .':'. round($lat, 3) .':'. $distance, 'cache_location', serialize($search_results));

  return $search_results;
}

/**
 * Helper function: this function is intended ONLY for use by _location_nearby_postalcodes().
 * It checks the cache for search-results on a given point and returns the appropriate subset
 * if one exists.
 *
 * @param $lon
 *   A floating point of the longitude coordinate of the search point
 *
 * @param $lat
 *   A floating point of the latitude coordinate of the search point
 *
 * @param $distance
 *   A floating point of the number of meters of the search radius
 *
 * @return
 *   An array of previous search results.
 *   If a previous search result on the same point was for the same search-radius, that result is returned.
 *   If a previous search result on the same point was for a longer search-radius, a subset of that result is returned.
 *   If a previous search result on the same point was for a shorter search-radius, the cached search-result is thrown out
 *   and an empty array is returned.
 *   If no previous search-result on the same point is in the cache, an empty array.
 */
function _location_search_results_from_cache($lon, $lat, $distance) {
  $cache_id_prefix = 'location_prox_search:'. round($lon, 3) .':'. round($lat, 3) .':';

  $result = db_query("SELECT cid FROM {cache_location} WHERE cid LIKE '%s%%'", $cache_id_prefix);

  if ($result_row = db_fetch_object($result)) {
    // A previous search has been done on the same search point, possibily with the an equal or different
    // search radius.
    $cached_key_fields = explode(':', $result_row->cid);
    $previous_search_radius = $cached_key_fields[3];

    // If the search-radius is less than or equal to the previous search-radius, then just use
    // the appropriate subset of the previous search result.
    // This is very convenient since previous search results are sorted in ascending order
    // by their distance from the search point.
    if ($distance <= $previous_search_radius) {

      $cached_search_results = cache_get($result_row->cid, 'cache_location');
      $cached_search_results = unserialize($cached_search_results->data);
      // If the cached-search had the exact same search-radius, just return the entire search result's
      // array from before,
      // otherwise, go through the distance-sorted search results and pick them out until the distances
      // of each search result start being something greater than the current search-radius
      if ($distance == $previous_search_radius) {
        return $cached_search_results;
      }
      else {
        $current_search_results = array();
        foreach ($cached_search_results as $key => $cached_result) {
          if ($cached_result['distance'] <= $distance) {
            $current_search_results[$key] = $cached_result;
          }
          else {
            break;
          }
        }
        return $current_search_results;
      }
    }
    else {
      // If the previous search-radius on the same point is smaller than the current search-radius,
      // then delete the previous search from the cache to make way in the cache for the results of
      // the current, more comprehensive search being done on the same point, but on a larger radius.
      // Return an empty array to let the calling function know that it will have to do a new search.

      cache_clear_all($result_row->cid, 'cache_location');
      return array();
    }

  }
  else {
    // This else-clause ties back to the first if-clause in this function.
    // It executes if no search has been done on this point.
    // If the {cache_location} table did not contain any useful cache id's, return the empty array.
    // This will let the calling function know that it has to do an original search.
    return array();
  }
}

/**
 * Helper function: This function EXPECTS an array that has been returned by
 *                  _location_sort_proximity_search_results()
 *
 * @param $results_array
 *   An array of proximity search results where
 *      -> the keys are integer rankings starting from 1 for the closest postal_code search result
 *      -> the values are associative arrays where
 *           'postal_code' => is the postal code of the search result
 *           'country'     => is the two-letter ISO of the country
 *           'lon'         => is the longitude coordinate of approximate center of the postal code
 *           'lat'         => is the latitude coordinate of the approximate center of the postal code
 *           'distance'    => is the distance of the approximate center of the result-postal_code from the
 *                            approximate center of the postal_code about which the search was executed.
 *                            This distance is in meters.
 *
 * @param $distance_unit
 *   A string abbreviation of a unit of distance.  This must be either 'mile' or 'km'.  If the argument is
 *   neither, it is forced to take the value of 'km'
 *
 * @return
 *   Returns nothing.
 *   Modifies parameter $results_array so that 'distance' (mentioned above) now points to a string representation
 *   of distance that goes to 1 decimal place, AFTER it has been converted
 *   from the original meters to the distance unit specified by $distance_unit.
 *   Also adds a key 'distance_unit' => 'mile' or 'km' which reflects the $distance_unit parameter.
 *
 */
function _location_format_search_result_distances(&$results_array, $distance_unit = 'km') {
  if ($distance_unit != 'km' && $distance_unit != 'mile') {
    $distance_unit = 'km';
  }

  // $conversion_factor = number to divide by to convert meters to $distance_unit
  // At this point, $distance_unit == 'km' or 'mile' and nothing else
  $conversion_factor = ($distance_unit == 'km') ? 1000.0 : 1609.347;

  foreach ($results_array as $index => $single_result) {
    $results_array[$index]['distance'] = round($single_result['distance']/$conversion_factor, 1);
  }
}

/**
 * Currently, this is not a priority until there is an implementable use for exact longitude,
 * latitude coordinates for an location.  The idea is that this call will eventually retrieve
 * information through a web-service.  Whereas location_latlon_rough() returns an approximate
 * lat/lon pair based strictly on the postal code where this lat/lon pair is pulled from a
 * database table, this function is intended to send the entire location to a web-service and
 * to retrieve exact lat/lon coordinates.
 *
 * @param $location
 *   An array where
 *     -> the key values are 'street', 'additional', 'province', 'country', 'postal_code'
 *     -> the values are:
 *         'street'         => the string representing the street location (REQUIRED)
 *         'additional'     => the string representing the additional street location portion in the location form
 *         'city'           => the city name (REQUIRED)
 *         'province'       => the province code defined in the country-specific include file
 *         'country'        => the lower-case of the two-letter ISO code (REQUIRED)
 *         'postal_code'    => the postal-code (REQUIRED)
 *
 * @return
 *   NULL if the delegated-to function that does the actual look-up does not exist.
 *   If the appropriate function exists, then this function returns an associative array where
 *      'lon' => A floating point number for the longitude coordinate of the parameter location
 *      'lat' => A floating point number for the latitude coordinate of the parameter location
 */
function location_latlon_exact($location = array()) {
  $country = $location['country'];
  location_standardize_country_code($country);
  $service = variable_get('location_geocode_'. $country, 'none');
  if (!empty($country) && $service != 'none') {
    // figure out what the exact function should be
    if (strpos($service, '|')) {
      location_load_country($country);
      // The code change below fixes the problem of the country specific
      // function for geocoding not being correctly called (it removes any
      // text from the pipe (|) onwards)
      $exact_latlon_function = 'location_geocode_'. $country .'_'. substr($service, 0, strpos($service, '|'));
    }
    else {
      $exact_latlon_function = $service .'_geocode_location';
    }
    if (function_exists($exact_latlon_function)) {
      return $exact_latlon_function($location);
    }
    else {
      return NULL;
    }
  }
  return NULL;
}

/**
 * @param $location
 *   An array where
 *     -> the key values are 'street', 'additional', 'province', 'country', 'postal_code'
 *     -> the values are:
 *         'street'         => the string representing the street location
 *         'additional'     => the string representing the additional street location portion in the location form
 *         'city'           => the city name
 *         'province'       => the province code defined in the country-specific include file
 *         'country'        => the lower-case of the two-letter ISO code
 *         'postal_code'    => the postal-code
 *         'lat'            => a floating-point of the latitude coordinate of this location if one is available
 *         'lon'            => a floating-point of the longitude coordinate of this location if one is available
 *
 * @return
 *   A string of HTML META tags to be include in the HTML head of an HTML page that geocodes
 *   as specifically as possible given the completeness of the parameter $location.  To be used
 *   to register location-specific content to search engines that crawl the site.
 */
function location_geocode_meta_tags($location = array()) {
  $output = "\n";
  if (isset($location['lat']) && isset($location['lon'])) {
    $output .= '<meta name=\'ICBM\' content="'. $location['lat'] .', '. $location['lon'] .'" />'."\n";
    $output .= '<meta name="geo.position" content="'. $location['lat'] .';'. $location['lon'] .'" />'."\n";
  }
  if (isset($location['country'])) {
    $output .= '<meta name="geo.region" content="'. strtoupper($location['country']);
    if (isset($location['province'])) {
      $output .= '-'. $location['province'];
    }
    $output .= '" />'."\n";
  }
  if (isset($location['city'])) {
    $output .= '<meta name="geo.placename" content="'. $location['city'] .'" />'."\n";
  }
  return $output;
}

/**
 * Returns an associative array of countries currently recognized by the
 * site admin's configuration where:
 *   -> the keys represent the two-letter ISO code and
 *   -> the values represent the English name of the country.
 *
 * The array is sorted by the values.
 *
 * Please note the difference between "supported" countries and "configured"
 * countries: A country being "supported" means that there is an include file
 * to support the country while "configured" implies that the site admin has
 * configured the site to actually use that country's include file.
 */
function location_configured_countries() {
  static $countries = array();
  if (empty($countries)) {
    $configured_countries = variable_get('location_configured_countries', array('us' => 'us')); // <integer index> => <ISO code>

    $configured_countries = is_array($configured_countries) ? $configured_countries : array();
    $supported_countries = _location_supported_countries(); // <ISO code> => <English name>

    foreach ($configured_countries as $country_code) {
      if (array_key_exists($country_code, $supported_countries)) {
        $countries[$country_code] = $supported_countries[$country_code];
      }
    }

    asort($countries);
  }
  return $countries;
}

/**
 * Returns an associative array of countries currently supported
 * by the location system where
 *   -> the keys represent the two-letter ISO code and
 *   -> the values represent the English name of the country.
 * The array is sorted the index (i.e., by the short English name of the country).
 *
 * Please note the different between "supported" countries and "configured"
 * countries: A country being "supported" means that there is an include file
 * to support the country while "configure" implies that the site admin has
 * configured the site to actually use that country.
 */
function _location_supported_countries() {
  static $supported_countries = array();

  // If this function has already been called this request, we can avoid a DB hit.
  if (!empty($supported_countries)) {
    return $supported_countries;
  }

  // Try first to load from cache, it's much faster than the scan below.
  if ($cache = cache_get('location:supported-countries', 'cache_location')) {
    $supported_countries = unserialize($cache->data);
  }
  else {
    // '<ISO two-letter code>' => '<English name for country>'
    $iso_list = location_get_iso3166_list();
    $path = drupal_get_path('module', 'location') .'/supported/location.';
    foreach ($iso_list as $cc => $name) {
      if (file_exists($path . $cc .'.inc')) {
        $supported_countries[$cc] = $name;
      }
    }
    cache_set('location:supported-countries', 'cache_location', serialize($supported_countries));
  }
  return $supported_countries;
}

/**
 * Given a pre-selected value, whether or not the form-item is required, and an optional form name,
 * generates an HTML select for selecting a country in an location form.
 *
 * @param $value
 *   A pre-selected value for this field.
 *
 * @param $required
 *   A boolean for whether this field is required or not.
 *
 * @param $form_name
 *   The HTML "name" attribute assigned to the generated form element.  Defaults to 'location' if none is specified
 *
 * @return
 *   A form_select where the labels are the English names of the countries and the corresponding value
 *   attributes for the options are the two-letter ISO code for the countries.
 *   If 'location' is passed as the $form_name, then the HTML "name" attribute for the generated form element
 *   will be "edit[location][country]".
 */
function _location_country_select_options($value = '', $required = FALSE, $function = NULL) {
  if ($function) {
    $function = ($function == 'nearby_postalcodes_bylocation' || $function == 'nearby_postalcodes_bylatlon') ? 'latlon_rough' : $function;
    $countrycodes = array();
    foreach (location_configured_countries() as $code => $full_name) {
      if (function_exists('location_'. $function .'_'. strtolower($code))) {
        $countrycodes[$code] = $full_name;
      }
    }
    $countrycodes = array_merge(array('' => ''), $countrycodes);
  }
  else {
    $countrycodes = array_merge(array('' => '', 'xx' => 'NOT LISTED'), location_get_iso3166_list());
  }
  return array(
    '#type'           => 'select',
    '#title'          => t('Country'),
    '#default_value'  => $value,
    '#options'        => $countrycodes,
    '#description'    => NULL,
    '#multiple'       => FALSE,
    '#required'       => $required
  );
}

// @@@ New in 3.x, document.
/**
 * Fetch the provinces for a country.
 */
function location_get_provinces($country = 'us') {
  static $provinces = array();
  location_standardize_country_code($country);
  if (isset($provinces[$country])) {
    return $provinces[$country];
  }
  if ($cache = cache_get("provinces:$country", 'cache_location')) {
    $provinces[$country] = unserialize($cache->data);
    return $provinces[$country];
  }
  location_load_country($country);
  $func = 'location_province_list_'. $country;
  if (function_exists($func)) {
    $provinces[$country] = $func();
    cache_set("provinces:$country", 'cache_location', serialize($provinces[$country]));
    return $provinces[$country];
  }
  return array();
}

// @@@ New in 3.x, document.
/**
 * Canonicalize a country code.
 */
function location_standardize_country_code(&$country) {
  $country = trim($country);
  // @@@ Double check the validity of this validity check. ;)
  if (!ctype_alpha($country) || strlen($country) != 2) {
    $country = 'xx';
    return FALSE;
  }
  else {
    $country = strtolower($country);
    return TRUE;
  }
}

// @@@ New in 3.x, document.
/**
 * Load the support file for a country.
 */
function location_load_country($country) {
  location_standardize_country_code($country);
  include_once(drupal_get_path('module', 'location') .'/supported/location.'. $country .'.inc');
}

/**
 * The array that is returned is a complete list of state/provinces
 * that belong to the countries enabled by the site's location system.
 *
 * @param $value
 *   A preselected value for the HTML select form item that is returned.
 *
 * @param $required
 *   A boolean for whether or not the field should be required.
 *
 * @param $country
 *   If you wish to only serve a state/province selection for a particular country while leaving out
 *   state/provinces by all other countries, pass the lower case of the country's two-letter ISO 3166
 *   code in this parameter.  If you wish to serve a province selection from all countries, pass NULL
 *   for this parameter.
 *
 *
 * @return
 *   An associative array where
 *     -> the keys are a contatenation of the countrycode and a three
 *        digit positive integer unique to each province within a
 *        country's provinces
 *     -> the values are the full name of the province
 *
 */
function location_province_select_options($country = NULL) {
  $options = array();
  $options['xx'] = t('NOT LISTED');

  if (isset($country)) {
    $countries = array($country => TRUE);
  }
  else {
    $countries = location_configured_countries();
dpm($countries);
  }
  foreach ($countries as $cc => $name) {
    $provinces = location_get_provinces($cc);
    // If we're listing multiple countries, do a divider.
    if (!$name === TRUE) {
      $obj = new stdClass();
      $obj->option = array('' => '[ ----- '. strtoupper($country_name) .' ----- ]');
      $options[] = $obj;
    }
    foreach ($provinces as $province_code => $province_name) {
      $obj = new stdClass();
      $obj->option = array("$cc-$province_code" => $province_name);
      $options[] = $obj;
    }
  }
  return $options;
}

/**
 * Create a single line address.
 *
 * @param $location
 *   Array. The address parts
 * @return
 *   String. The single line address
 */
function location_address2singleline($location = array()) {
  // Check if its a valid address
  if (empty($location)) {
    return '';
  }

  $address = '';
  if (!empty($location['street'])) {
    $address .= $location['street'];
  }

  if (!empty($location['city'])) {
    if (!empty($location['street'])) {
      $address .= ', ';
    }

    $address .= $location['city'];
  }

  if (!empty($location['province'])) {
    if (!empty($location['street']) || !empty($location['city'])) {
      $address .= ', ';
    }

    $address .= $location['province'];
  }

  if (!empty($location['postal_code'])) {
    if (!empty($address)) {
      $address .= ' ';
    }
    $address .= $location['postal_code'];
  }

  if (!empty($location['country'])) {
    $address .= ', '. $location['country'];
  }

  return $address;
}

function location_get_general_geocoder_list() {
  static $list;

  if (!count($list)) {
    $files = file_scan_directory(LOCATION_PATH .'/geocoding', '\.inc$', array('.', '..', 'CVS', '.svn'));
    foreach ($files as $full_path_name => $fileinfo) {
      $list[] = $fileinfo->name;
    }
  }

  return $list;
}

/**
 * The following is an array of all
 * countrycode => country-name pairs as layed out in
 * ISO 3166-1 alpha-2
 */
function location_get_iso3166_list($upper = FALSE) {
  $countries = array(
    'ad' => t('Andorra'),
    'ae' => t('United Arab Emirates'),
    'af' => t('Afghanistan'),
    'ag' => t('Antigua and Barbuda'),
    'ai' => t('Anguilla'),
    'al' => t('Albania'),
    'am' => t('Armenia'),
    'an' => t('Netherlands Antilles'),
    'ao' => t('Angola'),
    'aq' => t('Antarctica'),
    'ar' => t('Argentina'),
    'as' => t('American Samoa'),
    'at' => t('Austria'),
    'au' => t('Australia'),
    'aw' => t('Aruba'),
    'ax' => t('Aland Islands'),
    'az' => t('Azerbaijan'),
    'ba' => t('Bosnia and Herzegovina'),
    'bb' => t('Barbados'),
    'bd' => t('Bangladesh'),
    'be' => t('Belgium'),
    'bf' => t('Burkina Faso'),
    'bg' => t('Bulgaria'),
    'bh' => t('Bahrain'),
    'bi' => t('Burundi'),
    'bj' => t('Benin'),
    'bm' => t('Bermuda'),
    'bn' => t('Brunei'),
    'bo' => t('Bolivia'),
    'br' => t('Brazil'),
    'bs' => t('Bahamas'),
    'bt' => t('Bhutan'),
    'bv' => t('Bouvet Island'),
    'bw' => t('Botswana'),
    'by' => t('Belarus'),
    'bz' => t('Belize'),
    'ca' => t('Canada'),
    'cc' => t('Cocos (Keeling) Islands'),
    'cd' => t('Congo (Kinshasa)'),
    'cf' => t('Central African Republic'),
    'cg' => t('Congo (Brazzaville)'),
    'ch' => t('Switzerland'),
    'ci' => t('Ivory Coast'),
    'ck' => t('Cook Islands'),
    'cl' => t('Chile'),
    'cm' => t('Cameroon'),
    'cn' => t('China'),
    'co' => t('Colombia'),
    'cr' => t('Costa Rica'),
    'cs' => t('Serbia And Montenegro'),
    'cu' => t('Cuba'),
    'cv' => t('Cape Verde'),
    'cx' => t('Christmas Island'),
    'cy' => t('Cyprus'),
    'cz' => t('Czech Republic'),
    'de' => t('Germany'),
    'dj' => t('Djibouti'),
    'dk' => t('Denmark'),
    'dm' => t('Dominica'),
    'do' => t('Dominican Republic'),
    'dz' => t('Algeria'),
    'ec' => t('Ecuador'),
    'ee' => t('Estonia'),
    'eg' => t('Egypt'),
    'eh' => t('Western Sahara'),
    'er' => t('Eritrea'),
    'es' => t('Spain'),
    'et' => t('Ethiopia'),
    'fi' => t('Finland'),
    'fj' => t('Fiji'),
    'fk' => t('Falkland Islands'),
    'fm' => t('Micronesia'),
    'fo' => t('Faroe Islands'),
    'fr' => t('France'),
    'ga' => t('Gabon'),
    'gd' => t('Grenada'),
    'ge' => t('Georgia'),
    'gf' => t('French Guiana'),
    'gg' => t('Guernsey'),
    'gh' => t('Ghana'),
    'gi' => t('Gibraltar'),
    'gl' => t('Greenland'),
    'gm' => t('Gambia'),
    'gn' => t('Guinea'),
    'gp' => t('Guadeloupe'),
    'gq' => t('Equatorial Guinea'),
    'gr' => t('Greece'),
    'gs' => t('South Georgia and the South Sandwich Islands'),
    'gt' => t('Guatemala'),
    'gu' => t('Guam'),
    'gw' => t('Guinea-Bissau'),
    'gy' => t('Guyana'),
    'hk' => t('Hong Kong S.A.R., China'),
    'hm' => t('Heard Island and McDonald Islands'),
    'hn' => t('Honduras'),
    'hr' => t('Croatia'),
    'ht' => t('Haiti'),
    'hu' => t('Hungary'),
    'id' => t('Indonesia'),
    'ie' => t('Ireland'),
    'il' => t('Israel'),
    'im' => t('Isle of Man'),
    'in' => t('India'),
    'io' => t('British Indian Ocean Territory'),
    'iq' => t('Iraq'),
    'ir' => t('Iran'),
    'is' => t('Iceland'),
    'it' => t('Italy'),
    'je' => t('Jersey'),
    'jm' => t('Jamaica'),
    'jo' => t('Jordan'),
    'jp' => t('Japan'),
    'ke' => t('Kenya'),
    'kg' => t('Kyrgyzstan'),
    'kh' => t('Cambodia'),
    'ki' => t('Kiribati'),
    'km' => t('Comoros'),
    'kn' => t('Saint Kitts and Nevis'),
    'kp' => t('North Korea'),
    'kr' => t('South Korea'),
    'kw' => t('Kuwait'),
    'ky' => t('Cayman Islands'),
    'kz' => t('Kazakhstan'),
    'la' => t('Laos'),
    'lb' => t('Lebanon'),
    'lc' => t('Saint Lucia'),
    'li' => t('Liechtenstein'),
    'lk' => t('Sri Lanka'),
    'lr' => t('Liberia'),
    'ls' => t('Lesotho'),
    'lt' => t('Lithuania'),
    'lu' => t('Luxembourg'),
    'lv' => t('Latvia'),
    'ly' => t('Libya'),
    'ma' => t('Morocco'),
    'mc' => t('Monaco'),
    'md' => t('Moldova'),
    'me' => t('Montenegro'),
    'mg' => t('Madagascar'),
    'mh' => t('Marshall Islands'),
    'mk' => t('Macedonia'),
    'ml' => t('Mali'),
    'mm' => t('Myanmar'),
    'mn' => t('Mongolia'),
    'mo' => t('Macao S.A.R., China'),
    'mp' => t('Northern Mariana Islands'),
    'mq' => t('Martinique'),
    'mr' => t('Mauritania'),
    'ms' => t('Montserrat'),
    'mt' => t('Malta'),
    'mu' => t('Mauritius'),
    'mv' => t('Maldives'),
    'mw' => t('Malawi'),
    'mx' => t('Mexico'),
    'my' => t('Malaysia'),
    'mz' => t('Mozambique'),
    'na' => t('Namibia'),
    'nc' => t('New Caledonia'),
    'ne' => t('Niger'),
    'nf' => t('Norfolk Island'),
    'ng' => t('Nigeria'),
    'ni' => t('Nicaragua'),
    'nl' => t('Netherlands'),
    'no' => t('Norway'),
    'np' => t('Nepal'),
    'nr' => t('Nauru'),
    'nu' => t('Niue'),
    'nz' => t('New Zealand'),
    'om' => t('Oman'),
    'pa' => t('Panama'),
    'pe' => t('Peru'),
    'pf' => t('French Polynesia'),
    'pg' => t('Papua New Guinea'),
    'ph' => t('Philippines'),
    'pk' => t('Pakistan'),
    'pl' => t('Poland'),
    'pm' => t('Saint Pierre and Miquelon'),
    'pn' => t('Pitcairn'),
    'pr' => t('Puerto Rico'),
    'ps' => t('Palestinian Territory'),
    'pt' => t('Portugal'),
    'pw' => t('Palau'),
    'py' => t('Paraguay'),
    'qa' => t('Qatar'),
    're' => t('Reunion'),
    'ro' => t('Romania'),
    'rs' => t('Serbia'),
    'ru' => t('Russia'),
    'rw' => t('Rwanda'),
    'sa' => t('Saudi Arabia'),
    'sb' => t('Solomon Islands'),
    'sc' => t('Seychelles'),
    'sd' => t('Sudan'),
    'se' => t('Sweden'),
    'sg' => t('Singapore'),
    'sh' => t('Saint Helena'),
    'si' => t('Slovenia'),
    'sj' => t('Svalbard and Jan Mayen'),
    'sk' => t('Slovakia'),
    'sl' => t('Sierra Leone'),
    'sm' => t('San Marino'),
    'sn' => t('Senegal'),
    'so' => t('Somalia'),
    'sr' => t('Suriname'),
    'st' => t('Sao Tome and Principe'),
    'sv' => t('El Salvador'),
    'sy' => t('Syria'),
    'sz' => t('Swaziland'),
    'tc' => t('Turks and Caicos Islands'),
    'td' => t('Chad'),
    'tf' => t('French Southern Territories'),
    'tg' => t('Togo'),
    'th' => t('Thailand'),
    'tj' => t('Tajikistan'),
    'tk' => t('Tokelau'),
    'tl' => t('East Timor'),
    'tm' => t('Turkmenistan'),
    'tn' => t('Tunisia'),
    'to' => t('Tonga'),
    'tr' => t('Turkey'),
    'tt' => t('Trinidad and Tobago'),
    'tv' => t('Tuvalu'),
    'tw' => t('Taiwan'),
    'tz' => t('Tanzania'),
    'ua' => t('Ukraine'),
    'ug' => t('Uganda'),
    'uk' => t('United Kingdom'),
    'um' => t('United States Minor Outlying Islands'),
    'us' => t('United States'),
    'uy' => t('Uruguay'),
    'uz' => t('Uzbekistan'),
    'va' => t('Vatican'),
    'vc' => t('Saint Vincent and the Grenadines'),
    've' => t('Venezuela'),
    'vg' => t('British Virgin Islands'),
    'vi' => t('U.S. Virgin Islands'),
    'vn' => t('Vietnam'),
    'vu' => t('Vanuatu'),
    'wf' => t('Wallis and Futuna'),
    'ws' => t('Samoa'),
    'ye' => t('Yemen'),
    'yt' => t('Mayotte'),
    'za' => t('South Africa'),
    'zm' => t('Zambia'),
    'zw' => t('Zimbabwe'),
  );

  // In fact, the ISO codes for countries are all Upper Case.
  // So, if someone needs the list as the official records,
  // it will convert.
  if (!empty($upper)) {
    $countries = array_change_key_case($countries, CASE_UPPER);
  }
  // Sort the list and return
  natcasesort($countries);
  return $countries;
}
