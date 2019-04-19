
import React, {Component} from "react";
import PropTypes from 'prop-types';
import { withStyles } from '@material-ui/core/styles';

import Paper from '@material-ui/core/Paper';

import 'chartist/dist/chartist.min.css';
import Chartist from 'chartist';
import ChartistGraph from 'react-chartist';

const styles = theme => ({
  root: {
    textAlign: "center",
    verticalAlign: "middle"
  },
  chart: {
    height: "300px",
  },
  ctSeriesA: {
    color: 'blue',
  }
});

class LogGraph extends Component {
  constructor(props) {
    super(props);

    this.fetchLog = this.fetchLog.bind(this);
    this.state = {log: {timestamp_ms: []}}

    this.fetchLog();
  }

  fetchLog() {
    fetch('/api/log')
    .then(resp => resp.json())
    .then(
      (result) => {
        this.setState({log: result});
      },
      (error) => {
        console.log("error fetching log data", error);
      }
    )
    .finally(() => {
      this.timer = setTimeout(() => this.fetchLog(), 1000);
    })
  }

  getPlotData() {
    let timeOrigin = 0
    let logSize = this.state.log.timestamp_ms.length
    if(logSize > 0) {
      timeOrigin = this.state.log.timestamp_ms[0];
    }

    let series = [[], []];
    for(var i=0; i<logSize; i++) {
      const t = (this.state.log.timestamp_ms[i] - timeOrigin) / 1000.0
      series[0].push({x: t, y: this.state.log.targetTemperature[i]});
      series[1].push({x: t, y: this.state.log.measuredTemperature[i]});
    }
    return {series: series};
  }

  render() {
    
    let {classes} = this.props;

    const chartOptions = {
      axisX: {
        type: Chartist.AutoScaleAxis,
        onlyInteger: true
      },
      showPoint: false,
    }

    return <Paper className={classes.root}>
      <style dangerouslySetInnerHTML={{__html: `
        .ct-series-a .ct-line { stroke-dasharray: 10px 10px }
        .ct-series-b .ct-line { stroke: blue }
      `}} />
      <h2>Live View</h2>
      <ChartistGraph 
        className={classes.chart}
        options={chartOptions}
        data={this.getPlotData()}
        type='Line'
      />
    </Paper>
  }
}

LogGraph.propTypes = {
  classes: PropTypes.object.isRequired,
};

export default withStyles(styles)(LogGraph);