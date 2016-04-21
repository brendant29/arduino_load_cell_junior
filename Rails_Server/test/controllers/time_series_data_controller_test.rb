require 'test_helper'

class TimeSeriesDataControllerTest < ActionController::TestCase
  setup do
    @time_series_datum = time_series_data(:one)
  end

  test "should get index" do
    get :index
    assert_response :success
    assert_not_nil assigns(:time_series_data)
  end

  test "should get new" do
    get :new
    assert_response :success
  end

  test "should create time_series_datum" do
    assert_difference('TimeSeriesDatum.count') do
      post :create, time_series_datum: { datetime: @time_series_datum.datetime, lc1: @time_series_datum.lc1, lc2: @time_series_datum.lc2, lc3: @time_series_datum.lc3, lc4: @time_series_datum.lc4, station_id: @time_series_datum.station_id }
    end

    assert_redirected_to time_series_datum_path(assigns(:time_series_datum))
  end

  test "should show time_series_datum" do
    get :show, id: @time_series_datum
    assert_response :success
  end

  test "should get edit" do
    get :edit, id: @time_series_datum
    assert_response :success
  end

  test "should update time_series_datum" do
    patch :update, id: @time_series_datum, time_series_datum: { datetime: @time_series_datum.datetime, lc1: @time_series_datum.lc1, lc2: @time_series_datum.lc2, lc3: @time_series_datum.lc3, lc4: @time_series_datum.lc4, station_id: @time_series_datum.station_id }
    assert_redirected_to time_series_datum_path(assigns(:time_series_datum))
  end

  test "should destroy time_series_datum" do
    assert_difference('TimeSeriesDatum.count', -1) do
      delete :destroy, id: @time_series_datum
    end

    assert_redirected_to time_series_data_path
  end
end
