import React, { Component} from "react";
import PropTypes from 'prop-types';
import { withStyles } from '@material-ui/core/styles';
import Button from '@material-ui/core/Button';
import CircularProgress from '@material-ui/core/CircularProgress'
import Grid from '@material-ui/core/Grid';
import Paper from '@material-ui/core/Paper';
import Table from '@material-ui/core/Table';
import TableBody from '@material-ui/core/TableBody';
import TableCell from '@material-ui/core/TableCell';
import TableHead from '@material-ui/core/TableHead';
import TableRow from '@material-ui/core/TableRow';


const styles = theme => ({
  root: {
      width: '100%',
      marginTop: theme.spacing.unit * 3,
      overflowX: 'auto',
  },
  panel: {
  },
  stateLabelCell: {
      fontSize: '1.5em'
  },
  activeRow: {
    border: '3px solid #6bff97'
  },
  actionButton: {
    width: "100%"
  },
  paramCell: {
    textAlign: 'right'
  },
  paramInput: {
    width: '4em'
  }
});   


const SpinnerAdornment = (props => (
  <CircularProgress
    size={20}
  />
))

const AdornedButton = (props) => {
  const {
    children,
    loading,
    ...rest
  } = props
  return (
    <Button {...rest}>
      { !loading && children}
      {loading && <SpinnerAdornment {...rest} />}
    </Button>
  )
}

const capitalize = (s) => {
  if (typeof s !== 'string') return ''
  return s.charAt(0).toUpperCase() + s.slice(1)
}

class StateRow extends Component {
  
  constructor(props) {
    super(props);

    this.onClick = this.onClick.bind(this);
    this.paramChange = this.paramChange.bind(this);
    this.state = {actionPending: false, param: ''};
  }
  active() {
    return this.props.controllerState == this.props.targetState;
  }

  onClick() {
    let url = this.props.actionUrl;
    if(this.props.param) {
      url += "?" + this.props.param + "=" + this.state.param;
    }
    this.setState({actionPending: true});
    fetch(url)
    .then(
      (result) => {
        this.setState({actionPending: false});
        if(!result.ok) {
          this.setState({error: result.body});
          alert("Received bad response: " + result.statusText)
        }
      },
      (error) => {
        alert(error.message)
        this.setState({actionPending: false, error: error.message})
      }
    )
  }

  paramChange(event) {
    this.state.param = event.target.value;
  }

  
  render() {
    const { classes } = this.props;
    
    const paramElements = <label>
      {this.props.param}: <input className={classes.paramInput} value={this.state.value} onChange={this.paramChange} />
    </label>
      
    return(
      <TableRow className={this.active() ? classes.activeRow : ""}>
        <TableCell className={classes.stateLabelCell}>{capitalize(this.props.targetState)}</TableCell>
        <TableCell>{this.props.statusString}</TableCell>
        <TableCell className={classes.paramCell}>{this.props.param && paramElements}</TableCell>
        <TableCell>
          <AdornedButton
              className={classes.actionButton}
              variant="contained" 
              color="secondary" 
              disabled={this.state.actionPending} 
              loading={this.state.actionPending}
              onClick={this.onClick}>
            {this.props.actionLabel}
          </AdornedButton>
        </TableCell>
      </TableRow>
    )
  }
}

StateRow.propTypes = {
  classes: PropTypes.object.isRequired,
};

const StateRowStyles = withStyles(styles)(StateRow);

class ControlPanel extends Component {

  stopHandler() {
    this.setState({stopPending: true});
    fetch("/api/stop")
    .then(
      (result) => {
        this.setState({stopPending: false});
      },
      (error) => {
        this.setState({stopPending: false, stopError: error.message})
      }
    )
  }

  profileStatusString() {
    return 'Profile Stage: ' + this.props.controllerStatus.profileStage + 
            ', Elapsed: ' + this.props.controllerStatus.profileElapsedTime + 's';
  }

  render() {
    const { classes } = this.props;
    return (
      <Paper className={classes.panel}>
        <Table>
          <TableBody>
            <StateRowStyles 
                controllerState={this.props.controllerStatus.state}
                targetState="idle"
                actionUrl="/api/stop"
                actionLabel="Stop" />
            <StateRowStyles
                controllerState={this.props.controllerStatus.state}
                targetState="tempHold"
                actionUrl="/api/temphold"
                actionLabel="Set Temp"
                param="temp" />
            <StateRowStyles
                controllerState={this.props.controllerStatus.state}
                targetState="running"
                statusString={this.profileStatusString()}
                actionUrl="/api/start"
                actionLabel="Start Profile" />
            
            {/* <TableRow>
              <TableCell>Idle</TableCell>
              <TableCell></TableCell>
              <TableCell>
                <Button variant="contained" color="secondary" onClick={this.stopHandler}>
                  Stop
                </Button>
              </TableCell>
            </TableRow> */}
          </TableBody>
        </Table>
      </Paper>
    )
  }
}

export default withStyles(styles)(ControlPanel);