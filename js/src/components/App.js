import React, { Component } from "react";
import CssBaseline from '@material-ui/core/CssBaseline';
import Grid from '@material-ui/core/Grid';
import Paper from '@material-ui/core/Paper';
import { withStyles } from '@material-ui/core/styles';

import ControlPanel from './ControlPanel.js';
import LogGraph from './LogGraph.js';
import StatusBoard from './StatusBoard.js';
import ProfilePanel from './ProfilePanel.js';

const styles = theme => ({
  root: {
    flexGrow: 1,
    padding: '32px'
  },
  controlPanel: {
    width: "80%",
  },
  activeProfilePaper: {
    textAlign: "center"
  }
});

class App extends Component {
  constructor(props) {
    super(props);

    this.state = {
      controllerStatus: {
        state: "unknown"
      }
    }
    this.getStatus = this.getStatus.bind(this);
  }

  componentDidMount() {
    this.getStatus();
    this.getActiveProfile();
  }

  componentWillUnmount() {
    clearTimeout(this.timer);
    this.timer = null;
    clearTimeout(this.slowTimer);
    this.slowTimer = null;
  }

  getActiveProfile() {
    fetch('/api/profiles/active')
    .then(res => res.json())
    .then(
      (result) => {
        this.setState({activeProfile: result});
      },
      (error) => {
        console.log("Error loading active profile: ", error.message);
      }
    )
    .finally(() => {
      this.slowTimer = setTimeout(() => this.getActiveProfile(), 5000);
    })
  }

  getStatus() {
    fetch('/api/status')
    .then(res => res.json())
    .then(
      (result) => {
        this.setState({controllerStatus: result});
      },
      (error) => {
        console.log(error.message);
      }
    ).finally(() => {
      this.timer = setTimeout(() => this.getStatus(), 1000);
    })
  }
  
  render() {
    const {classes} = this.props;
    return <div>
        <CssBaseline />
        <Grid container className={classes.root} justify="center" spacing={16}>
          <Grid item className={classes.controlPanel} xs={6}>
              <ControlPanel controllerStatus={this.state.controllerStatus}></ControlPanel>
          </Grid>
          <Grid item xs={12}>
            <StatusBoard controllerStatus={this.state.controllerStatus}></StatusBoard>
          </Grid>
          <Grid item xs={8}>
            <ProfilePanel></ProfilePanel>
          </Grid>
          <Grid item xs={8}>
            <LogGraph />
          </Grid>
        </Grid>
      </div>
  }
}

export default withStyles(styles)(App);