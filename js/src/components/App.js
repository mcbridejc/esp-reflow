import React, { Component } from "react";
import CssBaseline from '@material-ui/core/CssBaseline';
import Grid from '@material-ui/core/Grid';
import { withStyles } from '@material-ui/core/styles';

import ControlPanel from './ControlPanel.js'

const styles = theme => ({
  root: {
    flexGrow: 1,
    padding: '32px'
  },
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
    this.timer = setTimeout(()=> this.getStatus(), 1000);
  }

  componentWillUnmount() {
    clearTimeout(this.timer);
    this.timer = null;
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
        <Grid container className={classes.root} spacing={16}>
          <Grid item xs={12}>
            <Grid container justify="center" spacing={16}>
              <ControlPanel controllerStatus={this.state.controllerStatus}></ControlPanel>
            </Grid>
          </Grid>
        </Grid>
      </div>
  }
}

export default withStyles(styles)(App);