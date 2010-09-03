/*********************************************************************
*
* Software License Agreement (BSD License)
*
*  Copyright (c) 2009, Willow Garage, Inc.
*  All rights reserved.
*
*  Redistribution and use in source and binary forms, with or without
*  modification, are permitted provided that the following conditions
*  are met:
*
*   * Redistributions of source code must retain the above copyright
*     notice, this list of conditions and the following disclaimer.
*   * Redistributions in binary form must reproduce the above
*     copyright notice, this list of conditions and the following
*     disclaimer in the documentation and/or other materials provided
*     with the distribution.
*   * Neither the name of Willow Garage, Inc. nor the names of its
*     contributors may be used to endorse or promote products derived
*     from this software without specific prior written permission.
*
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
*  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
*  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
*  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
*  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
*  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
*  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
*  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
*  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
*  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
*  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
*  POSSIBILITY OF SUCH DAMAGE.
*
* Author: Eitan Marder-Eppstein
*********************************************************************/
#include <dwa_local_planner/dwa_planner.h>
#include <angles/angles.h>

namespace dwa_local_planner {
  DWAPlanner::DWAPlanner(std::string name, costmap_2d::Costmap2DROS* costmap_ros) : costmap_ros_(NULL), world_model_(NULL) {
    costmap_ros_ = costmap_ros;
    costmap_ros_->getCostmapCopy(costmap_);

    map_ = base_local_planner::MapGrid(costmap_.getSizeInCellsX(), costmap_.getSizeInCellsY(), 
        costmap_.getResolution(), costmap_.getOriginX(), costmap_.getOriginY());
    ros::NodeHandle pn("~/" + name);

    double acc_lim_x, acc_lim_y, acc_lim_th;
    pn.param("acc_lim_x", acc_lim_x, 2.5);
    pn.param("acc_lim_y", acc_lim_y, 2.5);
    pn.param("acc_lim_th", acc_lim_th, 3.2);

    acc_lim_[0] = acc_lim_x;
    acc_lim_[1] = acc_lim_y;
    acc_lim_[2] = acc_lim_th;

    pn.param("max_vel_x", max_vel_x_, 0.6);
    pn.param("min_vel_x", min_vel_x_, 0.1);

    pn.param("max_vel_y", max_vel_y_, 0.1);
    pn.param("min_vel_y", min_vel_y_, -0.1);

    pn.param("min_trans_vel", min_vel_trans_, 0.1);

    pn.param("max_rotational_vel", max_vel_th_, 1.0);
    min_vel_th_ = -1.0 * max_vel_th_;

    pn.param("min_in_place_rotational_vel", min_in_place_vel_th_, 0.4);

    pn.param("sim_time", sim_time_, 1.5);
    pn.param("sim_granularity", sim_granularity_, 0.025);
    pn.param("path_distance_bias", pdist_scale_, 0.6);
    pn.param("goal_distance_bias", gdist_scale_, 0.8);
    pn.param("occdist_scale", occdist_scale_, 0.01);
    pn.param("heading_scale", heading_scale_, 1.0);

    pn.param("stop_time_buffer", stop_time_buffer_, 0.2);
    pn.param("oscillation_reset_dist", oscillation_reset_dist_, 0.05);
    pn.param("heading_lookahead", heading_lookahead_, 1.0);
    pn.param("rotation_lookahead", rotation_lookahead_, 0.325);

    pn.param("scaling_speed", scaling_speed_, 0.5);
    pn.param("max_scaling_factor", max_scaling_factor_, 0.5);

    int vx_samp, vy_samp, vth_samp;
    pn.param("vx_samples", vx_samp, 3);
    pn.param("vy_samples", vy_samp, 10);
    pn.param("vth_samples", vth_samp, 20);

    if(vx_samp <= 0){
      ROS_WARN("You've specified that you don't want to sample in the x dimension. We'll at least assume that you want to sample zero... so we're going to set vx_samples to 1 instead");
      vx_samp = 1;
    }

    if(vy_samp <= 0){
      ROS_WARN("You've specified that you don't want to sample in the y dimension. We'll at least assume that you want to sample zero... so we're going to set vy_samples to 1 instead");
      vy_samp = 1;
    }

    if(vth_samp <= 0){
      ROS_WARN("You've specified that you don't want to sample in the th dimension. We'll at least assume that you want to sample zero... so we're going to set vth_samples to 1 instead");
      vth_samp = 1;
    }

    pn.param("sim_period", sim_period_, 0.1);

    vsamples_[0] = vx_samp;
    vsamples_[1] = vy_samp;
    vsamples_[2] = vth_samp;

    footprint_spec_ = costmap_ros_->getRobotFootprint();

    world_model_ = new base_local_planner::CostmapModel(costmap_);

    prev_stationary_pos_ = Eigen::Vector3f::Zero();
    resetOscillationFlags();

  }

  Eigen::Vector3f DWAPlanner::computeNewPositions(const Eigen::Vector3f& pos, 
      const Eigen::Vector3f& vel, double dt){
    Eigen::Vector3f new_pos = Eigen::Vector3f::Zero();
    new_pos[0] = pos[0] + (vel[0] * cos(pos[2]) + vel[1] * cos(M_PI_2 + pos[2])) * dt;
    new_pos[1] = pos[1] + (vel[0] * sin(pos[2]) + vel[1] * sin(M_PI_2 + pos[2])) * dt;
    new_pos[2] = pos[2] + vel[2] * dt;
    return new_pos;
  }
  
  void DWAPlanner::selectBestTrajectory(base_local_planner::Trajectory* &best, base_local_planner::Trajectory* &comp){
    //check if the comp trajectory is better than the current best and, if so, swap them
    if(comp->cost_ >= 0.0 && (comp->cost_ < best->cost_ || best->cost_ < 0.0)){
      base_local_planner::Trajectory* swap = best;
      best = comp;
      comp = swap;
    }
  }

  void DWAPlanner::selectBestTrajectoryInPlaceRot(base_local_planner::Trajectory* &best, base_local_planner::Trajectory* &comp, double& best_heading_dist){
    //first, check if the trajectory scores comparably well
    if(comp->cost_ >= 0 && (comp->cost_ <= best->cost_ || best->cost_ < 0)){
      //next, to differentiate between rotations... we'll look ahead a bit
      double x_r, y_r, th_r;
      comp->getEndpoint(x_r, y_r, th_r);
      x_r += rotation_lookahead_ * cos(th_r);
      y_r += rotation_lookahead_ * sin(th_r);

      //get the goal distance of the associated cell
      unsigned int cell_x, cell_y; 
      if(costmap_.worldToMap(x_r, y_r, cell_x, cell_y)){
        double ahead_gdist = map_(cell_x, cell_y).goal_dist;
        //if this heading looks more promising than the last... take it
        if(ahead_gdist < best_heading_dist){
          base_local_planner::Trajectory* swap = best;
          best = comp;
          comp = swap;
          best_heading_dist = ahead_gdist;
        }
      }
    }
  }

  base_local_planner::Trajectory DWAPlanner::computeTrajectories(const Eigen::Vector3f& pos, const Eigen::Vector3f& vel){
    //get the index of the cell that we'll use for lookahead
    int lookahead_cell = getHeadingLookaheadIndex(heading_lookahead_, pos);

    //make sure that we have a valid cell to look at
    if(lookahead_cell < 0){
      base_local_planner::Trajectory t;
      t.cost_ = -1.0;
      return t;
    }

    geometry_msgs::PoseStamped heading_pose = global_plan_[lookahead_cell];

    //compute the feasible velocity space based on the rate at which we run
    Eigen::Vector3f max_vel = Eigen::Vector3f::Zero();
    max_vel[0] = std::max(std::min(max_vel_x_, vel[0] + acc_lim_[0] * sim_period_), min_vel_trans_);
    max_vel[1] = std::max(std::min(max_vel_y_, vel[1] + acc_lim_[1] * sim_period_), min_vel_trans_);
    max_vel[2] = std::max(std::min(max_vel_th_, vel[2] + acc_lim_[2] * sim_period_), min_vel_th_);

    Eigen::Vector3f min_vel = Eigen::Vector3f::Zero();
    min_vel[0] = std::min(std::max(min_vel_x_, vel[0] - acc_lim_[0] * sim_period_), -1.0 * min_vel_trans_);
    min_vel[1] = std::min(std::max(min_vel_y_, vel[1] - acc_lim_[1] * sim_period_), -1.0 * min_vel_trans_);
    min_vel[2] = std::min(std::max(min_vel_th_, vel[2] - acc_lim_[2] * sim_period_), max_vel_th_);

    Eigen::Vector3f dv = Eigen::Vector3f::Zero();
    //we want to sample the velocity space regularly
    for(unsigned int i = 0; i < 3; ++i){
      if(vsamples_[i] > 1)
        dv[i] = (max_vel[i] - min_vel[i]) / (vsamples_[i] - 1);
    }

    //keep track of the best trajectory seen so far... we'll re-use two memeber vars for efficiency
    base_local_planner::Trajectory* best_traj = &traj_one_;
    best_traj->cost_ = -1.0;

    base_local_planner::Trajectory* comp_traj = &traj_two_;
    comp_traj->cost_ = -1.0;

    Eigen::Vector3f vel_samp = Eigen::Vector3f::Zero();

    //ROS_ERROR("x(%.2f, %.2f), y(%.2f, %.2f), th(%.2f, %.2f)", min_vel[0], max_vel[0], min_vel[1], max_vel[1], min_vel[2], max_vel[2]);
    //ROS_ERROR("x(%.2f, %.2f), y(%.2f, %.2f), th(%.2f, %.2f)", min_vel_x_, max_vel_x_, min_vel_y_, max_vel_y_, min_vel_th_, max_vel_th_);
    //ROS_ERROR("dv %.2f %.2f %.2f", dv[0], dv[1], dv[2]);

    //first... we'll make sure to simulate the case where y and th are zero
    vel_samp[0] = min_vel[0];
    for(unsigned int i = 0; i < vsamples_[0]; ++i){
      //we won't sample trajectories that don't meet the minimum speed requirements
      if(fabs(vel_samp[0]) < min_vel_trans_){
        vel_samp[0] += dv[0];
        continue;
      }

      //simulate the trajectory and select it if its better
      if((vel_samp[0] < 0 && !forward_pos_only_) || (vel_samp[0] > 0 && !forward_neg_only_)){
        generateTrajectory(pos, vel_samp, heading_pose, *comp_traj);
        selectBestTrajectory(best_traj, comp_traj);
        vel_samp[0] += dv[0];
      }
    }

    vel_samp[0] = min_vel[0];
    //Next, we'll just loop through the trajectories as normal
    for(unsigned int i = 0; i < vsamples_[0]; ++i){
      //we only want to sample x velocities that are legal given our oscillation flags
      if((vel_samp[0] < 0 && !forward_pos_only_) || (vel_samp[0] > 0 && !forward_neg_only_)){
        //we only want to set the y velocity if we're sampling more than one point
        double local_dy = dv[1];
        if(vsamples_[1] > 1){
          //we want to limit our y velocity further based on the x speed we're sampling
          //this will keep the robot from selecting a x/y velocity pair that makes the 
          //robot go sideways towards the goal as the desired trajectory

          //we'll define an ellipse centered at 0... going to the maximum x and y speeds respectively
          //double abs_limited_y = yFromElipse(max_vel[0], max_vel[1], vel_samp[0]);
          double abs_limited_y = yFromElipse(max_vel_x_, max_vel_y_, vel_samp[0]);

          //we'll bound the minimum and maximum velocities and compute a step
          double local_y_min = std::max(-1.0 * abs_limited_y, (double)min_vel[1]);
          double local_y_max = std::min(abs_limited_y, (double)max_vel[1]);
          local_dy  = (local_y_max - local_y_min) / (vsamples_[1] - 1);

          vel_samp[1] = local_y_min;

          //ROS_ERROR("%.2f: %.2f, %.2f, %.2f -- %.2f", vel_samp[0], local_y_min, local_y_max, local_dy, abs_limited_y);
        }
        else
          vel_samp[1] = 0.0;

        for(unsigned int j = 0; j < vsamples_[1]; ++j){
          //we won't sample trajectories that don't meet the minimum translational speed requirements
          double vel_sq = vel_samp[0] * vel_samp[0] + vel_samp[1] * vel_samp[1];
          if(fabs(vel_samp[0]) < min_vel_trans_){
            vel_samp[0] += local_dy;
            continue;
          }

          //we only want to set the th velocity if we're sampling more than one point
          if(vsamples_[2] > 1)
            vel_samp[2] = min_vel[2];
          else
            vel_samp[2] = 0.0;

          for(unsigned int k = 0; k < vsamples_[2]; ++k){
            //simulate the trajectory and select it if its better
            generateTrajectory(pos, vel_samp, heading_pose, *comp_traj);
            selectBestTrajectory(best_traj, comp_traj);
            vel_samp[2] += dv[2];
          }
          vel_samp[1] += local_dy;
        }
      }
      vel_samp[0] += dv[0];
    }

    //we want to make sure to simulate strafing without motion in the x direction
    if(vsamples_[1] > 1){
      vel_samp[0] = 0.0;
      vel_samp[1] = min_vel[1];
      vel_samp[2] = 0.0;

      for(unsigned int i = 0; i < vsamples_[1]; ++i){
        //we won't sample trajectories that don't meet the minimum speed requirements
        if(fabs(vel_samp[1]) < min_vel_trans_){
          vel_samp[1] += dv[1];
          continue;
        }

        //we'll only simulate negative strafes if we haven't comitted to positive strafes
        if(vel_samp[1] < 0 && !strafe_pos_only_){
          //simulate the trajectory and select it if its better
          generateTrajectory(pos, vel_samp, heading_pose, *comp_traj);
          selectBestTrajectory(best_traj, comp_traj);
        }
        //we'll make the inverse check for positive strafing
        else if(!strafe_neg_only_){
          //simulate the trajectory and select it if its better
          generateTrajectory(pos, vel_samp, heading_pose, *comp_traj);
          selectBestTrajectory(best_traj, comp_traj);
        }
        vel_samp[1] += dv[1];
      }
    }

    //and we have the special case where we want to simulate in-place rotations
    if(vsamples_[2] > 1){
      vel_samp[0] = 0.0;
      vel_samp[1] = 0.0;
      vel_samp[2] = min_vel[2];

      double best_heading_gdist = DBL_MAX;

      for(unsigned int i = 0; i < vsamples_[2]; ++i){
        //we won't sample trajectories that don't meet the minimum speed requirements
        if(fabs(vel_samp[2]) < min_in_place_vel_th_){
          vel_samp[2] += dv[2];
          continue;
        }

        //we'll only simulate negative rotations if we haven't comitted to positive rotations
        if(vel_samp[1] < 0 && !rot_pos_only_){
          //simulate the trajectory and select it if its better
          generateTrajectory(pos, vel_samp, heading_pose, *comp_traj);
          selectBestTrajectoryInPlaceRot(best_traj, comp_traj, best_heading_gdist);
        }
        //we'll make the inverse check for positive rotations
        else if(!rot_neg_only_){
          //simulate the trajectory and select it if its better
          generateTrajectory(pos, vel_samp, heading_pose, *comp_traj);
          selectBestTrajectoryInPlaceRot(best_traj, comp_traj, best_heading_gdist);
        }
        vel_samp[2] += dv[2];
      }
    }

    //ok... now we have our best trajectory
    if(best_traj->cost_ >= 0){
      //we want to check if we need to set any oscillation flags
      if(setOscillationFlags(best_traj)){
        prev_stationary_pos_ = pos;
      }

      //if we've got restrictions... check if we can reset any oscillation flags
      if(forward_pos_only_ || forward_neg_only_ 
         || strafe_pos_only_ || strafe_neg_only_
	 || rot_pos_only_ || rot_neg_only_){
        resetOscillationFlagsIfPossible(pos, prev_stationary_pos_);
      }
    }

    //TODO: Think about whether we want to try to do things like back up when a valid trajectory is not found

    return *best_traj;

  }

  void DWAPlanner::resetOscillationFlagsIfPossible(const Eigen::Vector3f& pos, const Eigen::Vector3f& prev){
    double x_diff = pos[0] - prev[0];
    double y_diff = pos[1] - prev[1];
    double sq_dist = x_diff * x_diff + y_diff * y_diff;

    //if we've moved far enough... we can reset our flags
    if(sq_dist > oscillation_reset_dist_ * oscillation_reset_dist_){
      resetOscillationFlags();
    }
  }

  void DWAPlanner::resetOscillationFlags(){
    strafe_pos_only_ = false;
    strafe_neg_only_ = false;
    strafing_pos_ = false;
    strafing_neg_ = false;

    rot_pos_only_ = false;
    rot_neg_only_ = false;
    rotating_pos_ = false;
    rotating_neg_ = false;

    forward_pos_only_ = false;
    forward_neg_only_ = false;
    forward_pos_ = false;
    forward_neg_ = false;
  }
  
  bool DWAPlanner::setOscillationFlags(base_local_planner::Trajectory* t){
    bool flag_set = false;
    //set oscillation flags for moving forward and backward
    if(t->xv_ < 0.0){
      if(forward_pos_){
        forward_neg_only_ = true;
	flag_set = true;
      }
      forward_pos_ = false;
      forward_neg_ = true;
    }
    if(t->xv_ > 0.0){
      if(forward_neg_){
        forward_pos_only_ = true;
	flag_set = true;
      }
      forward_neg_ = false;
      forward_pos_ = true;
    }

    //we'll only set flags for strafing and rotating when we're not moving forward at all
    if(fabs(t->xv_) <= min_vel_trans_){
      //check negative strafe
      if(t->yv_ < 0){
        if(strafing_pos_){
          strafe_neg_only_ = true;
	  flag_set = true;
	}
        strafing_pos_ = false;
        strafing_neg_ = true;
      }

      //check positive strafe
      if(t->yv_ > 0){
        if(strafing_neg_){
          strafe_pos_only_ = true;
	  flag_set = true;
	}
        strafing_neg_ = false;
        strafing_pos_ = true;
      }

      //check negative rotation
      if(t->thetav_ < 0){
        if(rotating_pos_){
          rot_neg_only_ = true;
	  flag_set = true;
	}
        rotating_pos_ = false;
        rotating_neg_ = true;
      }

      //check positive rotation
      if(t->thetav_ > 0){
        if(rotating_neg_){
          rot_pos_only_ = true;
	  flag_set = true;
	}
        rotating_neg_ = false;
        rotating_pos_ = true;
      }
    }
    return flag_set;
  }

  double DWAPlanner::footprintCost(const Eigen::Vector3f& pos, double scale){
    double cos_th = cos(pos[2]);
    double sin_th = sin(pos[2]);

    std::vector<geometry_msgs::Point> scaled_oriented_footprint;
    for(unsigned int i  = 0; i < footprint_spec_.size(); ++i){
      geometry_msgs::Point new_pt;
      new_pt.x = pos[0] + (scale * footprint_spec_[i].x * cos_th - scale * footprint_spec_[i].y * sin_th);
      new_pt.y = pos[1] + (scale * footprint_spec_[i].x * sin_th + scale * footprint_spec_[i].y * cos_th);
      scaled_oriented_footprint.push_back(new_pt);
    }

    geometry_msgs::Point robot_position;
    robot_position.x = pos[0];
    robot_position.y = pos[1];

    //check if the footprint is legal
    double footprint_cost = world_model_->footprintCost(robot_position, scaled_oriented_footprint, costmap_.getInscribedRadius(), costmap_.getCircumscribedRadius());

    return footprint_cost;
  }

  void DWAPlanner::generateTrajectory(Eigen::Vector3f pos, const Eigen::Vector3f& vel, const geometry_msgs::PoseStamped& heading_pose, base_local_planner::Trajectory& traj){
    //ROS_ERROR("%.2f, %.2f, %.2f - %.2f %.2f", vel[0], vel[1], vel[2], sim_time_, sim_granularity_);
    double impossible_cost = map_.map_.size();

    double vmag = sqrt(vel[0] * vel[0] + vel[1] * vel[1]);

    //compute the number of steps we must take along this trajectory to be "safe"
    int num_steps = ceil(std::max((vmag * sim_time_) / sim_granularity_, fabs(vel[2]) / sim_granularity_));

    //compute a timestep
    double dt = sim_time_ / num_steps;
    double time = 0.0;

    //compute the time we would stop at applying max decelleration after the next period
    double stop_time = sim_period_ + getStopTime(vel);
    double heading_diff = M_PI;

    //initialize the costs for the trajectory
    double path_dist = 0.0;
    double goal_dist = 0.0;
    double occ_cost = 0.0;

    //create a potential trajectory... it might be reused so we'll make sure to reset it
    traj.resetPoints();
    traj.xv_ = vel[0];
    traj.yv_ = vel[1];
    traj.thetav_ = vel[2];
    traj.cost_ = -1.0;

    //if we're not actualy going to simulate... we may as well just return now
    if(num_steps == 0){
      return;
    }

    //we want to check against the absolute value of the velocities for collisions later
    Eigen::Vector3f abs_vel = vel.cwise().abs();

    //simulate the trajectory and check for collisions, updating costs along the way
    for(int i = 0; i < num_steps; ++i){
      //get the mapp coordinates of a point
      unsigned int cell_x, cell_y;

      //we won't allow trajectories that go off the map... shouldn't happen that often anyways
      if(!costmap_.worldToMap(pos[0], pos[1], cell_x, cell_y)){
        //we're off the map
        traj.cost_ = -1.0;
        return;
      }

      //if we're over a certain speed threshold, we'll scale the robot's
      //footprint to make it either slow down or stay further from walls
      double scale = 1.0;
      if(vel[0] > scaling_speed_){
        //scale up to the max scaling factor linearly... this could be changed later
        double ratio = (vel[0] - scaling_speed_) / (max_vel_x_ - scaling_speed_);
        scale = max_scaling_factor_ * ratio + 1.0;
      }

      //we want to find the cost of the footprint
      double footprint_cost = footprintCost(pos, scale);

      //if the footprint hits an obstacle... we'll check if we can stop before we hit it... given the time to get there
      if(footprint_cost < 0){
        //we want to compute the max allowable speeds to be able to stop
        //to be safe... we'll make sure we can stop some time before we actually hit
        Eigen::Vector3f max_vel = getMaxSpeedToStopInTime(time - stop_time_buffer_);

        //check if we can stop in time
        if(abs_vel[0] < max_vel[0] && abs_vel[1] < max_vel[1] && abs_vel[2] < max_vel[2]){
          //if we can, then we'll just break out of the loop here.. no point in checking future points
          break;
        }
        else{
          //if we can't... then this trajectory is invalid
          traj.cost_ = -1.0;
          return;
        }
      }

      //compute the costs for this point on the trajectory
      occ_cost = std::max(std::max(occ_cost, footprint_cost), double(costmap_.getCost(cell_x, cell_y)));
      path_dist = map_(cell_x, cell_y).path_dist;
      goal_dist = map_(cell_x, cell_y).goal_dist;

      //check if we want to compute heading distance
      if(time > stop_time && time < stop_time + dt){
        heading_diff = headingDiff(heading_pose.pose.position.x, heading_pose.pose.position.y, pos);
      }

      //if a point on this trajectory has no clear path to the goal... it is invalid
      if(impossible_cost <= goal_dist || impossible_cost <= path_dist){
        traj.cost_ = -2.0; //-2.0 means that we were blocked because propagation failed
        return;
      }

      //add the point to the trajectory so we can draw it later if we want
      traj.addPoint(pos[0], pos[1], pos[2]);

      //update the position of the robot using the velocities passed in
      pos = computeNewPositions(pos, vel, dt);
      time += dt;
    }

    double velocity_mag = sqrt(vel[0] * vel[0] + vel[1] * vel[1]);

    double normalized_heading = heading_diff / M_PI;
    double normalized_vel = (1.0 - velocity_mag / sqrt(max_vel_x_ * max_vel_x_ + max_vel_y_ * max_vel_y_));
    double normalized_occ_cost = occ_cost / costmap_2d::LETHAL_OBSTACLE;
    double normalized_gdist = goal_dist / impossible_cost;
    double normalized_pdist = path_dist / impossible_cost;

    //compute the final cost
    //traj.cost_ = gdist_scale_ * normalized_gdist + pdist_scale_ * normalized_pdist + occdist_scale_* normalized_occ_cost; 
    //traj.cost_ = 1.0 * normalized_heading + gdist_scale_ * normalized_gdist; // +   0.2 * normalized_vel + 0.2 * normalized_occ_cost;
    traj.cost_ = heading_scale_ * normalized_heading + pdist_scale_ * path_dist + gdist_scale_ * goal_dist + occdist_scale_ * occ_cost;
    //ROS_ERROR("%.2f, %.2f, %.2f, %.2f", vel[0], vel[1], vel[2], traj.cost_);
  }

  bool DWAPlanner::checkTrajectory(const Eigen::Vector3f& pos, const Eigen::Vector3f& vel){
    //get the index of the cell that we'll use for lookahead
    int lookahead_cell = getHeadingLookaheadIndex(heading_lookahead_, pos);

    geometry_msgs::PoseStamped heading_pose;

    //make sure that we have a valid cell to look at
    if(lookahead_cell >= 0){
      heading_pose = global_plan_[lookahead_cell];
    }

    base_local_planner::Trajectory t;
    generateTrajectory(pos, vel, heading_pose, t);

    //if the trajectory is a legal one... the check passes
    if(t.cost_ >= 0)
      return true;

    //otherwise the check fails
    return false;
  }

  void DWAPlanner::updatePlan(const std::vector<geometry_msgs::PoseStamped>& new_plan){
    global_plan_.resize(new_plan.size());
    for(unsigned int i = 0; i < new_plan.size(); ++i){
      global_plan_[i] = new_plan[i];
    }
  }

  //given the current state of the robot, find a good trajectory
  base_local_planner::Trajectory DWAPlanner::findBestPath(tf::Stamped<tf::Pose> global_pose, tf::Stamped<tf::Pose> global_vel, 
      tf::Stamped<tf::Pose>& drive_velocities){

    //make sure to get an updated copy of the costmap before computing trajectories
    costmap_ros_->getCostmapCopy(costmap_);

    Eigen::Vector3f pos(global_pose.getOrigin().getX(), global_pose.getOrigin().getY(), tf::getYaw(global_pose.getRotation()));
    Eigen::Vector3f vel(global_vel.getOrigin().getX(), global_vel.getOrigin().getY(), tf::getYaw(global_vel.getRotation()));

    //reset the map for new operations
    map_.resetPathDist();

    //make sure that we update our path based on the global plan and compute costs
    map_.setPathCells(costmap_, global_plan_);
    ROS_DEBUG("Path/Goal distance computed");

    //rollout trajectories and find the minimum cost one
    base_local_planner::Trajectory best = computeTrajectories(pos, vel);
    ROS_DEBUG("Trajectories created");

    //if we don't have a legal trajectory, we'll just command zero
    if(best.cost_ < 0){
      drive_velocities.setIdentity();
    }
    else{
      btVector3 start(best.xv_, best.yv_, 0);
      drive_velocities.setOrigin(start);
      btMatrix3x3 matrix;
      matrix.setRotation(tf::createQuaternionFromYaw(best.thetav_));
      drive_velocities.setBasis(matrix);
    }

    return best;
  }

  double DWAPlanner::headingDiff(double gx, double gy, const Eigen::Vector3f& pos){
    Eigen::Vector2f v1(gx - pos[0], gy -pos[1]);
    Eigen::Vector2f v2(cos(pos[2]), sin(pos[2]));

    v1.normalize();
    v2.normalize();
    double dot = v1.dot(v2);
    double perp_dot = v1[0] * v2[1] - v1[1] * v2[0];
    return atan2(perp_dot, dot);

    //double v1_x = gx - pos[0];
    //double v1_y = gy - pos[1];
    //double v2_x = cos(pos[2]);
    //double v2_y = sin(pos[2]);

    //double perp_dot = v1_x * v2_y - v1_y * v2_x;
    //double dot = v1_x * v2_x + v1_y * v2_y;

    ////get the signed angle
    //double vector_angle = atan2(perp_dot, dot);

    //double heading_diff = abs(vector_angle);
    //return heading_diff;
  }

  int DWAPlanner::getHeadingLookaheadIndex(double lookahead_dist, const Eigen::Vector3f& pos){
    // move back on the global plan until we reach the first point within the
    // distance of the robot
    for(int i = global_plan_.size() - 1; i >= 0; --i){
      double sq_dist = (pos[0] - global_plan_[i].pose.position.x) * (pos[0] - global_plan_[i].pose.position.x) 
        + (pos[1] - global_plan_[i].pose.position.y) * (pos[1] - global_plan_[i].pose.position.y);
      if(sq_dist <= (lookahead_dist * lookahead_dist)){
        return i;
      }
    }
    return -1;
  }
};
