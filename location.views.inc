<?php
// $Id$

/**
 * @file
 * Views 2 support for Location.
 */

/*

TODO:
 * Finish porting!
 * Write "relationships" -- see book.views.inc, upload.views.inc, nodequeue...


*/

/**
 * Implementation of hook_views_handlers().
 */
function location_views_handlers() {
  return array(
    'info' => array(
      'path' => drupal_get_path('module', 'location') .'/handlers',
    ),
    'handlers' => array(
      'location_views_handler_field_latitude' => array(
        'parent' => 'views_handler_field',
      ),
      'location_views_handler_field_longitude' => array(
        'parent' => 'location_views_handler_field_latitude',
      ),
      'location_views_handler_field_coordinates' => array(
        'parent' => 'location_views_handler_field_latitude',
      ),
      'location_handler_field_location_country' => array(
        'parent' => 'views_handler_field',
      ),
      'location_handler_field_location_province' => array(
        'parent' => 'views_handler_field',
      ),
      'location_views_handler_filter_proximity' => array(
        'parent' => 'views_handler_filter',
      ),
    ),
  );
}

/**
 * Implementation of hook_views_data().
 */
function location_views_data() {
  // ----------------------------------------------------------------
  // location table -- basic table information.

  // Define the base group of this table. Fields that don't
  // have a group defined will go into this field by default.

  $data['location']['table']['group'] = t('Location');

  // Advertise this table as a possible base table
  $data['location']['table']['base'] = array(
    'field' => 'lid',
    'title' => t('Location'),
    'help' => t("Locations are addresses and map coordinates."),
    'weight' => -10,
  );

  $data['location']['table']['join'] = array(
    // Location links to node through location_instance via lid.
    'node' => array(
      'left_table' => 'location_instance',
      'left_field' => 'lid',
      'field' => 'lid',
    ),
    // Location links to node_revisions through location_instance via lid.
    'node_revisions' => array(
      'table' => 'location_instance',
      'left_field' => 'lid',
      'field' => 'lid',
    ),
    // Location links to users through location_instance via lid.
    'users' => array(
      'table' => 'location_instance',
      'left_field' => 'lid',
      'field' => 'lid',
    ),
  );

  // ----------------------------------------------------------------
  // location table -- fields

  // lid
  $data['location']['lid'] = array(
    'title' => t('Lid'),
    'help' => t('The location ID of the location.'), // The help that appears on the UI,
    // Information for displaying the lid
    'field' => array(
      'handler' => 'views_handler_field', // @@@
      'click sortable' => TRUE,
    ),
    // Information for accepting a lid as an argument
    /*
    'argument' => array(
      'handler' => 'views_handler_argument_node_nid',
      'name field' => 'title', // the field to display in the summary.
      'numeric' => TRUE,
      'validate type' => 'nid',
    ),
    */
    // Information for accepting a lid as a filter
    'filter' => array(
      'handler' => 'views_handler_filter_numeric',
    ),
    // Information for sorting on a lid.
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
  );

  $data['location']['name'] = array(
    'title' => t('Name'),
    'field' => array(
      'click sortable' => TRUE,
    ),
  );
  $data['location']['street'] = array(
    'title' => t('Street'),
    'field' => array(
      'click sortable' => TRUE,
    ),
  );
  $data['location']['additional'] = array(
    'title' => t('Additional'),
    'field' => array(
      'click sortable' => TRUE,
    ),
  );
  $data['location']['city'] = array(
    'title' => t('City'),
    'field' => array(
      'click sortable' => TRUE,
    ),
  );

  // @@@ 1.x Conversion -- 'province' => 'province', style 'name'; 'province_code' => 'province', style 'code'
  $data['location']['province'] = array(
    'title' => t('Province'),
    'field' => array(
      'handler' => 'location_handler_field_location_province',
      'click sortable' => TRUE,
    ),
  );

  $data['location']['postal_code'] = array(
    'title' => t('Postal Code'),
    'field' => array(
      'click sortable' => TRUE,
    ),
  );

  // @@@ 1.x Conversion -- 'country' => 'country', style 'name'; 'country_code' => 'country', style 'code'.
  $data['location']['country'] = array(
    'title' => t('Country'),
    'field' => array(
      'handler' => 'location_handler_field_location_country',
      'click sortable' => TRUE,
    ),
  );

  $data['location']['latitude'] = array(
    'title' => t('Latitude'),
    'field' => array(
      'handler' => 'location_views_handler_field_latitude',
      'click sortable' => TRUE,
    ),
  );
  $data['location']['longitude'] = array(
    'title' => t('Longitude'),
    'field' => array(
      'handler' => 'location_views_handler_field_longitude',
      'click sortable' => TRUE,
    ),
  );
  $data['location']['coordinates'] = array(
    'title' => t('Coordinates'),
    'field' => array(
      'field' => 'latitude', // The handler adds the longitude.
      'handler' => 'location_views_handler_field_coordinates',
      'click sortable' => FALSE,
    ),
    'filter' => array(
      'handler' => 'location_views_handler_filter_proximity',
    ),
  );

/*
      'latitude' => array(
        'name' => t('Latitude'),
        'sortable' => TRUE,
      ),
      'longitude' => array(
        'name' => t('Longitude'),
        'sortable' => TRUE,
      ),
*/




  $data['location_instance']['table']['group']  = t('Location');

  $data['location_instance']['table']['join'] = array(
    'location' => array(
      'left_field' => 'lid',
      'field' => 'lid',
    ),
    'users' => array(
      'left_field' => 'uid',
      'field' => 'uid',
    ),
    'node' => array(
      'left_field' => 'vid',
      'field' => 'vid',
    ),
    'node_revisions' => array(
      'left_field' => 'vid',
      'field' => 'vid',
    ),
  );


  // Tell the base tables about us.
  $data['node']['table']['join']['location'] = array(
    'left_table' => 'location_instance',
    'left_field' => 'vid',
    'right_field' => 'vid',
  );
  $data['node_revisions']['table']['join']['location'] = array(
    'left_table' => 'location_instance',
    'left_field' => 'vid',
    'right_field' => 'vid',
  );
  $data['users']['table']['join']['location'] = array(
    'left_table' => 'location_instance',
    'left_field' => 'vid',
    'right_field' => 'vid',
  );

  return $data;
}
