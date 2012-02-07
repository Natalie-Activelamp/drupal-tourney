<?php

/**
 * @file
 * Default plugin for a default style.
 */

/**
* Single Elimination Tournament
*/
$plugin = array(
  'name' => t('Double Elimination Tournament'),
  'machine name' => 'double_elim',
  'description' => t('This is the description of the tournament'),
  'weight' => 1,
  'total games' => 5,
  'builder controller' => 'DoubleElimination', 
  'completed controller' => 'DoubleEliminationMatchComplete',
);

// Define any classes below to extend default functionality in the controllers.
// The default implementation works for this plugin.