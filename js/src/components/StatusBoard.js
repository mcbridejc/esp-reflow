import React, {Component} from "react";
import PropTypes from 'prop-types';
import { withStyles } from '@material-ui/core/styles';
import Grid from '@material-ui/core/Grid';
import Paper from '@material-ui/core/Paper';


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
  }
});

class StatusBoard extends Component {
  constructor(props) {
    super(props);
  }

  render() { 
    const { classes } = this.props;
    return (
      <Grid container className={classes.root} justify="center" spacing={16}>
        <Grid item xs={2}> 
          <Paper className={classes.paper}>
            <div className={classes.gaugeLabel}>Temperature</div>
            <div className={classes.gaugeValue}>{this.props.controllerStatus.temperature}</div>
          </Paper>
        </Grid>
        <Grid item xs={2}> 
          <Paper className={classes.paper}>
            <div className={classes.gaugeLabel}>Target</div>
            <div className={classes.gaugeValue}>{this.props.controllerStatus.targetTemperature}</div>
          </Paper>
        </Grid>
        <Grid item xs={2}> 
          <Paper className={classes.paper}>
            <div className={classes.gaugeLabel}>Ouptut</div>
            <div className={classes.gaugeValue}>{this.props.controllerStatus.output}</div>
          </Paper>
        </Grid>

      </Grid>
    );
  }
}

StatusBoard.propTypes = {
  classes: PropTypes.object.isRequired,
};

export default withStyles(styles)(StatusBoard);