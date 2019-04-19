import React, {Component} from "react";
import PropTypes from 'prop-types';
import { withStyles } from '@material-ui/core/styles';
import Grid from '@material-ui/core/Grid';
import Paper from '@material-ui/core/Paper';

import 'chartist/dist/chartist.min.css';
import Chartist from 'chartist';
import ChartistGraph from 'react-chartist';

const styles = theme => ({
  root: {},
  paper: {
    textAlign: "center",
    verticalAlign: "middle",
  },
  gaugeLabel: {
    fontSize: "1.1em",
    paddingTop: "1em"
  },
  gaugeValue: {
    fontSize: "1.5em",
    fontWeight: "bold",
    padding: "1em",
  },
  chart: {
    height: "300px",
  }
});

class ProfileGraph extends Component {
  constructor(props) {
    super(props);

    this.profileData = this.profileData.bind(this);
  }

  profileData() {
    let values = []
    let lastTemp = 50;
    let time = 0;
    
    if(this.props.profile && this.props.profile.steps) {
      this.props.profile.steps.forEach((step) => {
        if(step.ramp == 1) {
          values.push({x: time, y: lastTemp});
        } else {
          values.push({x: time, y: step.temp});
        }
        time += step.duration;
        lastTemp = step.temp;
        values.push({x: time, y: step.temp});
      });
    }
    return {
      series: [values]
    };
  }

  render() {
    const {classes} = this.props;

    const chartOptions = {
      axisX: {
        type: Chartist.AutoScaleAxis,
        onlyInteger: true
      },
      lineSmooth: Chartist.Interpolation.none({
        fillHoles: false
      })
    }


    return (
      <ChartistGraph className={classes.chart} options={chartOptions} data={this.profileData()} type={'Line'} />
    )
  }
  
}

ProfileGraph.propTypes = {
  classes: PropTypes.object.isRequired,
  profile: PropTypes.object
};

export default withStyles(styles)(ProfileGraph);